//
// Created by 冯昊 on 2023/9/1.
//

#include "gc_driver.h"

GCDriver::GCDriver(const std::string& binaryFile, uint8_t device_id) {
  // Index calculation
  // The get_xil_devices will return vector of Xilinx Devices
  std::vector<cl::Device> devices = xcl::get_xil_devices();
  /* Multi board support: selecting the right device based on the device_id,
   * provided through command line args (-id <device_id>).
   */
  if (devices.size() <= device_id) {
    std::cout << "Identfied devices = " << devices.size()
              << ", given device id = " << unsigned(device_id) << std::endl;
    std::cout << "Error: Device ID should be within the range of number of "
                 "Devices identified"
              << std::endl;
    std::cout << "Program exited..\n" << std::endl;
    exit(1);
  }
  devices.at(0) = devices.at(device_id);

  cl::Device device = devices.at(0);

  // Creating Context and Command Queue for selected Device
  m_context = new cl::Context(device);
  m_q = new cl::CommandQueue(
      *m_context, device,
      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
  std::string device_name = device.getInfo<CL_DEVICE_NAME>();
  std::cout << "Found Device=" << device_name.c_str()
            << ", device id = " << unsigned(device_id) << std::endl;

  // import_binary() command will find the OpenCL binary file created using the
  // v++ compiler load into OpenCL Binary and return as Binaries
  // OpenCL and it can contain many functions which can be executed on the
  // device.

  auto fileBuf = xcl::read_binary_file(binaryFile.c_str());
  cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
  devices.resize(1);

  m_program = new cl::Program(*m_context, devices, bins);
}

GCDriver::~GCDriver() {
  delete (m_program);
  delete (m_q);
  delete (m_context);
}

void GCDriver::run_gc_kernel(
    const std::vector<std::string>& input_filenames,
    const std::string& output_filename,
    const std::vector<std::pair<size_t, unsigned char*>>& bitmaps,
    const std::vector<std::uint64_t>& input_entries, uint8_t cu_index,
    std::vector<std::pair<std::string, std::vector<uint32_t>>>* rewrite_keys,
    std::vector<uint64_t>& output_meta) {
  int err;
  size_t input_size = input_filenames.size();

  cl::Buffer* bufInputFiles;
  cl::Buffer* bufInputBitmaps;
  cl::Buffer* bufOutputFile;
  cl::Buffer* bufOutputKey;
  cl::Buffer* bufOutputMeta;
  cl::Buffer* bufIntputLength;
  cl::Buffer* bufIntputBitmapLength;
  cl::Buffer* bufIntputEntries;

  cl_mem_ext_ptr_t inExt, outExt;
  inExt = {XCL_MEM_EXT_P2P_BUFFER, nullptr, nullptr};
  outExt = {XCL_MEM_EXT_P2P_BUFFER, nullptr, nullptr};

  // Device buffer allocation for input files
  size_t input_files_size = 0;
  std::vector<uint64_t, aligned_allocator<uint64_t>> file_sizes(input_size, 0);
  std::vector<uint64_t, aligned_allocator<uint64_t>> num_entries(input_size, 0);
  for (size_t i = 0; i < input_size; ++i) {
    size_t file_size = get_file_size(input_filenames[i].c_str());
    file_size = ((file_size >> 20) + 1) << 20;
    file_sizes[i] = file_size;
    input_files_size += file_size;
    num_entries[i] = input_entries[i];
  }
  OCL_CHECK(err, bufInputFiles = new cl::Buffer(
                     *m_context, CL_MEM_READ_ONLY | CL_MEM_EXT_PTR_XILINX,
                     input_files_size, &inExt, &err))
  uint64_t input_offset = 0;
  for (size_t i = 0; i < input_size; ++i) {
    int fd_p2p_in = open(input_filenames[i].c_str(), O_RDWR | O_DIRECT);
    uint64_t size = file_sizes[i];

    if (fd_p2p_in < 0) {
      std::cerr << "ERROR: open " << input_filenames[i]
                << "failed: " << std::endl;
      exit(EXIT_FAILURE);
    }

    cl::Event p2pReadEvent;
    OCL_CHECK(err, char* p2pPtr = (char*)m_q->enqueueMapBuffer(
                       *bufInputFiles, CL_TRUE, CL_MAP_WRITE, input_offset,
                       size, nullptr, &p2pReadEvent, &err))
    OCL_CHECK(err, err = p2pReadEvent.wait())
    long buf_size = 128 * 1024 * 1024;  // 128MB
    long read_offset = 0;
    auto p2p_read_start = std::chrono::system_clock::now();
    for (size_t iter = 0; iter < size / buf_size + 1; ++iter) {
      auto ret = pread(fd_p2p_in, (void*)p2pPtr, buf_size, read_offset);
      if (ret <= 0) {
        std::cerr << "ERROR: read " << input_filenames[i] << " failed: " << ret
                  << ", size:" << size << ", line: " << __LINE__
                  << ", errno:" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
      }
      read_offset += ret;
      p2pPtr += buf_size;
    }
    auto p2p_read_end = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now() - p2p_read_start);
    std::cout << "INFO: Successfully transfer data(byte): " << read_offset
              << ", time(s): " << p2p_read_end.count() << std::endl;
    (void)close(fd_p2p_in);
    input_offset += size;
  }

  // Device buffer allocation for bitmap
  std::vector<uint64_t, aligned_allocator<uint64_t>> bitmap_sizes(input_size,
                                                                  0);
  size_t input_bitmaps_size = 0;
  for (size_t i = 0; i < input_size; ++i) {
    bitmap_sizes[i] = bitmaps[i].first;
    input_bitmaps_size += bitmap_sizes[i];
  }
  input_bitmaps_size = ((input_bitmaps_size >> 20) + 1) << 20;
  auto bitmaps_tmp = (unsigned char*)aligned_alloc(4096, input_bitmaps_size);
  memset(bitmaps_tmp, 0, input_bitmaps_size);
  input_offset = 0;
  for (const auto& item : bitmaps) {
    memcpy(bitmaps_tmp + input_offset, item.second, item.first);
    input_offset += item.first;
  }
  OCL_CHECK(err, bufInputBitmaps = new cl::Buffer(
                     *m_context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                     input_bitmaps_size, bitmaps_tmp, &err))

  // Device buffer allocation for output files
  size_t output_file_size = MAX_DATA_SIZE * sizeof(unsigned char);
  OCL_CHECK(err, bufOutputFile = new cl::Buffer(
                     *m_context, CL_MEM_WRITE_ONLY | CL_MEM_EXT_PTR_XILINX,
                     output_file_size, &outExt, &err))
  cl::Event p2pWritePrepareEvent;
  void* output_p2p_ptr =
      m_q->enqueueMapBuffer(*bufOutputFile, CL_TRUE, CL_MAP_READ, 0,
                            output_file_size, nullptr, &p2pWritePrepareEvent, &err);
  OCL_CHECK(err, err = p2pWritePrepareEvent.wait())

  // Device buffer allocation for output key
  auto output_key_tmp = (unsigned char*)aligned_alloc(
      4096, MAX_DATA_SIZE * sizeof(unsigned char));
  memset(output_key_tmp, 0, MAX_DATA_SIZE * sizeof(unsigned char));
  OCL_CHECK(err,
            bufOutputKey = new cl::Buffer(
                *m_context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                MAX_DATA_SIZE * sizeof(unsigned char), output_key_tmp, &err))

  // Device buffer allocation for output meta
  std::vector<uint64_t, aligned_allocator<uint64_t>> output_meta_tmp(
      OUTPUT_META_SIZE, 0);
  OCL_CHECK(err, bufOutputMeta = new cl::Buffer(
                     *m_context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                     OUTPUT_META_SIZE * sizeof(uint64_t),
                     output_meta_tmp.data(), &err))

  // Device buffer allocation for input length
  OCL_CHECK(err,
            bufIntputLength = new cl::Buffer(
                *m_context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                file_sizes.size() * sizeof(uint64_t), file_sizes.data(), &err))
  // Device buffer allocation for input bitmap length
  OCL_CHECK(err, bufIntputBitmapLength = new cl::Buffer(
                     *m_context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                     bitmap_sizes.size() * sizeof(uint64_t),
                     bitmap_sizes.data(), &err))
  // Device buffer allocation for input entry num
  OCL_CHECK(
      err, bufIntputEntries = new cl::Buffer(
               *m_context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
               num_entries.size() * sizeof(uint64_t), num_entries.data(), &err))

  cl::Event objMigrateEvent;
  m_q->enqueueMigrateMemObjects({*bufInputBitmaps, *bufIntputLength,
                                 *bufIntputBitmapLength, *bufIntputEntries},
                                0 /* 0 means from host*/, nullptr, &objMigrateEvent);
  OCL_CHECK(err, err = objMigrateEvent.wait())
  delete bitmaps_tmp;

  std::string gc_kernel_name = gc_kernel_names[cu_index];
  cl::Kernel gcKernel(*m_program, gc_kernel_name.c_str());
  cl::Event kernelFinishEvent, outputFinishEvent;
  std::vector<cl::Event> writeWait;
  int narg = 0;
  gcKernel.setArg(narg++, *bufInputFiles);
  gcKernel.setArg(narg++, *bufInputBitmaps);
  gcKernel.setArg(narg++, *bufOutputFile);
  gcKernel.setArg(narg++, *bufOutputKey);
  gcKernel.setArg(narg++, *bufOutputMeta);
  gcKernel.setArg(narg++, *bufIntputLength);
  gcKernel.setArg(narg++, *bufIntputBitmapLength);
  gcKernel.setArg(narg++, *bufIntputEntries);
  gcKernel.setArg(narg++, input_size);

  auto kernel_start = std::chrono::system_clock::now();
  std::cout << "Kernel task start. " << std::endl;
  m_q->enqueueTask(gcKernel, nullptr, &kernelFinishEvent);
  writeWait.push_back(kernelFinishEvent);
  m_q->enqueueMigrateMemObjects({*bufOutputKey, *bufOutputMeta},
                                CL_MIGRATE_MEM_OBJECT_HOST, &writeWait,
                                &outputFinishEvent);
  outputFinishEvent.wait();
  auto kernel_time_s = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now() - kernel_start);
  std::cout << "Kernel task finished. Time used: " << kernel_time_s.count()
            << "s" << std::endl;

  int fd_p2p_out =
      open(output_filename.c_str(), O_CREAT | O_RDWR | O_DIRECT, 0777);
  if (fd_p2p_out <= 0) {
    std::cout << "P2P: Unable to open output file, exited!, ret: " << fd_p2p_out
              << std::endl;
    close(fd_p2p_out);
    exit(1);
  }
  auto p2p_write_start = std::chrono::system_clock::now();
  auto ret = write(fd_p2p_out, output_p2p_ptr, output_meta_tmp[1]);
  if (ret <= 0) {
    std::cerr << "ERROR: write " << output_filename << " failed: " << ret
              << ", size:" << output_meta_tmp[1] << ", line: " << __LINE__
              << ", errno:" << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
  auto p2p_write_end = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now() - p2p_write_start);
  std::cout << "INFO: Successfully transfer data(byte): " << ret
            << ", key num: " << output_meta_tmp[0]
            << ", time(s): " << p2p_write_end.count() << std::endl;
  close(fd_p2p_out);
  delete bufInputFiles;
  delete bufInputBitmaps;
  delete bufOutputFile;
  delete bufOutputKey;
  delete bufOutputMeta;
  delete bufIntputLength;
  delete bufIntputBitmapLength;
  delete bufIntputEntries;

  uint64_t cur_key_offset = 0;
  for (size_t i = 0; i < output_meta_tmp[0]; ++i) {
    rewrite_keys->push_back(GetOutputKey(output_key_tmp, &cur_key_offset));
  }
  for (const auto& item : output_meta_tmp) {
    output_meta.push_back(item);
  }

  delete output_key_tmp;
}

std::pair<std::string, std::vector<uint32_t>> GCDriver::GetOutputKey(
    unsigned char* data, uint64_t* cur_offset) {
  data += *cur_offset;
  uint32_t key_size = 0;
  uint64_t offset = 0;
  std::vector<uint32_t> decode_result(4);
  DecodeVarint32(data, key_size, offset);
  std::string key(reinterpret_cast<char*>(data) + offset, key_size);
  DecodeFixed32(data + offset + key_size, &decode_result[0]);
  DecodeFixed32(data + offset + key_size + 4, &decode_result[1]);
  DecodeFixed32(data + offset + key_size + 8, &decode_result[2]);
  DecodeFixed32(data + offset + key_size + 12, &decode_result[3]);
  *cur_offset += offset + key_size + 16;
  return std::make_pair(key, decode_result);
}
