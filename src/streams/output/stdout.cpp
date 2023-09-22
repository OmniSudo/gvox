#include <gvox/stream.h>
#include <gvox/streams/output/stdout.h>

#include <iostream>
#include <string_view>

struct GvoxStdoutOutputStream {
    explicit GvoxStdoutOutputStream(GvoxStdoutOutputStreamConfig const &config);

    static auto write(uint8_t *data, size_t size) -> GvoxResult;
};

GvoxStdoutOutputStream::GvoxStdoutOutputStream(GvoxStdoutOutputStreamConfig const & /*unused*/) {
}

auto GvoxStdoutOutputStream::write(uint8_t *data, size_t size) -> GvoxResult {
    auto *chars = reinterpret_cast<char *>(data);
    std::cout << std::string_view{chars, chars + size};
    return GVOX_SUCCESS;
}

auto gvox_output_stream_stdout_create(void **self, GvoxOutputStreamCreateCbArgs const *args) -> GvoxResult {
    GvoxStdoutOutputStreamConfig config;
    if (args->config != nullptr) {
        config = *static_cast<GvoxStdoutOutputStreamConfig const *>(args->config);
    } else {
        config = {};
    }
    *self = new GvoxStdoutOutputStream(config);
    return GVOX_SUCCESS;
}
auto gvox_output_stream_stdout_write(void *self, GvoxOutputStream /*unused*/, uint8_t *data, size_t size) -> GvoxResult {
    return static_cast<GvoxStdoutOutputStream *>(self)->write(data, size);
}
auto gvox_output_stream_stdout_seek(void * /*unused*/, GvoxOutputStream /*unused*/, long /*unused*/, GvoxSeekOrigin /*unused*/) -> GvoxResult {
    return GVOX_ERROR_UNKNOWN;
}
auto gvox_output_stream_stdout_tell(void * /*unused*/, GvoxOutputStream /*unused*/) -> long {
    return GVOX_ERROR_UNKNOWN;
}
void gvox_output_stream_stdout_destroy(void *self) {
    std::cout << std::flush;
    delete static_cast<GvoxStdoutOutputStream *>(self);
}
