
set(HEADER_CONTENT "#pragma once

#include <gvox/gvox.h>
#include <string_view>
#include <array>
#include <utility>

using StdInputAdapterInfo     = std::pair<std::string_view, GvoxInputAdapterDescription>;
using StdOutputAdapterInfo    = std::pair<std::string_view, GvoxOutputAdapterDescription>;
using StdParserInfo           = std::pair<std::string_view, GvoxParserDescription>;
using StdSerializerInfo       = std::pair<std::string_view, GvoxSerializerDescription>;
using StdContainerInfo        = std::pair<std::string_view, GvoxContainerDescription>;
")

foreach(NAME ${GVOX_INPUT_ADAPTERS})
    target_sources(${PROJECT_NAME} PRIVATE "src/adapters/input/${NAME}.cpp")
    set(HEADER_CONTENT "${HEADER_CONTENT}
    auto gvox_input_adapter_${NAME}_create(void **self, GvoxInputAdapterCreateCbArgs const *args) -> GvoxResult;
    auto gvox_input_adapter_${NAME}_read(void *self, GvoxInputAdapter next_handle, uint8_t *data, size_t size) -> GvoxResult;
    auto gvox_input_adapter_${NAME}_seek(void *self, GvoxInputAdapter next_handle, long offset, GvoxSeekOrigin origin) -> GvoxResult;
    auto gvox_input_adapter_${NAME}_tell(void *self, GvoxInputAdapter next_handle) -> long;
    void gvox_input_adapter_${NAME}_destroy(void *self);
    ")
    set(INPUT_ADAPTER_INFOS_CONTENT "${INPUT_ADAPTER_INFOS_CONTENT}
    StdInputAdapterInfo{
        \"${NAME}\",
        {
            .create  = gvox_input_adapter_${NAME}_create,
            .read    = gvox_input_adapter_${NAME}_read,
            .seek    = gvox_input_adapter_${NAME}_seek,
            .tell    = gvox_input_adapter_${NAME}_tell,
            .destroy = gvox_input_adapter_${NAME}_destroy,
        },
    },")
endforeach()

foreach(NAME ${GVOX_OUTPUT_ADAPTERS})
    target_sources(${PROJECT_NAME} PRIVATE "src/adapters/output/${NAME}.cpp")
    set(HEADER_CONTENT "${HEADER_CONTENT}
    auto gvox_output_adapter_${NAME}_create(void **self, GvoxOutputAdapterCreateCbArgs const *) -> GvoxResult;
    auto gvox_output_adapter_${NAME}_write(void *self, GvoxOutputAdapter next_handle, uint8_t *data, size_t size) -> GvoxResult;
    auto gvox_output_adapter_${NAME}_seek (void *self, GvoxOutputAdapter next_handle, long offset, GvoxSeekOrigin origin) -> GvoxResult;
    auto gvox_output_adapter_${NAME}_tell (void *self, GvoxOutputAdapter next_handle) -> long;
    void gvox_output_adapter_${NAME}_destroy(void *self);
    ")
    set(OUTPUT_ADAPTER_INFOS_CONTENT "${OUTPUT_ADAPTER_INFOS_CONTENT}
    StdOutputAdapterInfo{
        \"${NAME}\",
        {
            .create  = gvox_output_adapter_${NAME}_create,
            .write   = gvox_output_adapter_${NAME}_write,
            .seek    = gvox_output_adapter_${NAME}_seek,
            .tell    = gvox_output_adapter_${NAME}_tell,
            .destroy = gvox_output_adapter_${NAME}_destroy,
        },
    },")
endforeach()

foreach(NAME ${GVOX_PARSERS})
    target_sources(${PROJECT_NAME} PRIVATE "src/parsers/${NAME}.cpp")
    set(HEADER_CONTENT "${HEADER_CONTENT}
    auto gvox_parser_${NAME}_create(void **self, GvoxParserCreateCbArgs const *args) -> GvoxResult;
    void gvox_parser_${NAME}_destroy(void *self);
    auto gvox_parser_${NAME}_create_from_input(GvoxInputAdapter input_adapter, GvoxParser *user_parser) -> GvoxResult;
    ")
    set(PARSER_INFOS_CONTENT "${PARSER_INFOS_CONTENT}
    StdParserInfo{
        \"${NAME}\",
        {
            .create  = gvox_parser_${NAME}_create,
            .destroy = gvox_parser_${NAME}_destroy,
            .create_from_input = gvox_parser_${NAME}_create_from_input,
        },
    },")
endforeach()

foreach(NAME ${GVOX_SERIALIZERS})
    target_sources(${PROJECT_NAME} PRIVATE "src/serializers/${NAME}.cpp")
    set(HEADER_CONTENT "${HEADER_CONTENT}
    auto gvox_serializer_${NAME}_create(void **self, GvoxSerializerCreateCbArgs const *args) -> GvoxResult;
    void gvox_serializer_${NAME}_destroy(void *self);
    ")
    set(SERIALIZER_INFOS_CONTENT "${SERIALIZER_INFOS_CONTENT}
    StdSerializerInfo{
        \"${NAME}\",
        {
            .create  = gvox_serializer_${NAME}_create,
            .destroy = gvox_serializer_${NAME}_destroy,
        },
    },")
endforeach()

foreach(NAME ${GVOX_CONTAINERS})
    target_sources(${PROJECT_NAME} PRIVATE "src/containers/${NAME}.cpp")
    set(HEADER_CONTENT "${HEADER_CONTENT}
    auto gvox_container_${NAME}_create(void **self, GvoxContainerCreateCbArgs const *args) -> GvoxResult;
    void gvox_container_${NAME}_destroy(void *self);
    ")
    set(CONTAINER_INFOS_CONTENT "${CONTAINER_INFOS_CONTENT}
    StdContainerInfo{
        \"${NAME}\",
        {
            .create    = gvox_container_${NAME}_create,
            .destroy   = gvox_container_${NAME}_destroy,
        },
    },")
endforeach()

set(HEADER_CONTENT "${HEADER_CONTENT}
static constexpr auto standard_input_adapters = std::array{${INPUT_ADAPTER_INFOS_CONTENT}
    StdInputAdapterInfo{\"null\", {}},
};
static constexpr auto standard_output_adapters = std::array{${OUTPUT_ADAPTER_INFOS_CONTENT}
    StdOutputAdapterInfo{\"null\", {}},
};
static constexpr auto standard_parsers = std::array{${PARSER_INFOS_CONTENT}
    StdParserInfo{\"null\", {}},
};
static constexpr auto standard_serializers = std::array{${SERIALIZER_INFOS_CONTENT}
    StdSerializerInfo{\"null\", {}},
};
static constexpr auto standard_containers = std::array{${CONTAINER_INFOS_CONTENT}
    StdContainerInfo{\"null\", {}},
};
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gvox_standard_functions.hpp" "${HEADER_CONTENT}")