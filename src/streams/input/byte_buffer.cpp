#include <gvox/gvox.h>
#include <gvox/streams/input/byte_buffer.h>

#include <vector>
#include <algorithm>

struct GvoxByteBufferInputStream {
    std::vector<uint8_t> bytes{};
    long current_read_head{};

    GvoxByteBufferInputStream(GvoxByteBufferInputStreamConfig const &config);

    auto read(uint8_t *data, size_t size) -> GvoxResult;
    auto seek(long offset, GvoxSeekOrigin origin) -> GvoxResult;
};

GvoxByteBufferInputStream::GvoxByteBufferInputStream(GvoxByteBufferInputStreamConfig const &config) {
    bytes.resize(config.size);
    std::copy(config.data, config.data + config.size, bytes.begin());
}

auto GvoxByteBufferInputStream::read(uint8_t *data, size_t size) -> GvoxResult {
    auto position = current_read_head;
    current_read_head += size;
    if (current_read_head > bytes.size()) {
        return GVOX_ERROR_UNKNOWN;
    }
    std::copy(bytes.data() + position, bytes.data() + position + size, static_cast<uint8_t *>(data));
    return GVOX_SUCCESS;
}

auto GvoxByteBufferInputStream::seek(long offset, GvoxSeekOrigin origin) -> GvoxResult {
    switch (origin) {
    case GVOX_SEEK_ORIGIN_BEG: current_read_head = 0 + offset; break;
    case GVOX_SEEK_ORIGIN_END: current_read_head = bytes.size() + offset; break;
    case GVOX_SEEK_ORIGIN_CUR: current_read_head = current_read_head + offset; break;
    default: break;
    }
    return GVOX_SUCCESS;
}

GvoxResult gvox_input_stream_byte_buffer_create(void **self, void const *config_ptr) {
    GvoxByteBufferInputStreamConfig config;
    if (config_ptr) {
        config = *static_cast<GvoxByteBufferInputStreamConfig const *>(config_ptr);
    } else {
        config = {};
    }
    *self = new GvoxByteBufferInputStream(config);
    return GVOX_SUCCESS;
}

GvoxResult gvox_input_stream_byte_buffer_read(void *self, uint8_t *data, size_t size) {
    return static_cast<GvoxByteBufferInputStream *>(self)->read(data, size);
}

GvoxResult gvox_input_stream_byte_buffer_seek(void *self, long offset, GvoxSeekOrigin origin) {
    return static_cast<GvoxByteBufferInputStream *>(self)->seek(offset, origin);
}

void gvox_input_stream_byte_buffer_destroy(void *self) {
    delete static_cast<GvoxByteBufferInputStream *>(self);
}
