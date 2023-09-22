#include <gvox/gvox.h>
#include <gvox/containers/bounded_raw.h>

#include "../utils/tracy.hpp"
#include "gvox/core.h"

#include <cstdlib>
#include <cstdint>

#include <array>
#include <vector>
#include <span>
#include <new>
#include <bit>
#include <unordered_map>
#include <concepts>

#include <iostream>
#include <iomanip>

#include "raw_helper.hpp"

struct GvoxBoundedRawContainer {
    GvoxVoxelDesc voxel_desc{};
    GvoxExtent extent{};
    void *pre_allocated_buffer{};

    std::vector<int64_t> pre_alloc_offset_buffer{};
};

auto gvox_container_bounded_raw_create(void **out_self, GvoxContainerCreateCbArgs const *args) -> GvoxResult {
    GvoxBoundedRawContainerConfig config;
    if (args->config != nullptr) {
        config = *static_cast<GvoxBoundedRawContainerConfig const *>(args->config);
    } else {
        config = {};
    }
    (*out_self) = new GvoxBoundedRawContainer({
        .voxel_desc = config.voxel_desc,
        .extent = config.extent,
        .pre_allocated_buffer = config.pre_allocated_buffer,
        .pre_alloc_offset_buffer = std::vector<int64_t>(static_cast<size_t>(config.extent.axis_n) * 3),
    });
    return GVOX_SUCCESS;
}
void gvox_container_bounded_raw_destroy(void *self_ptr) {
    delete static_cast<GvoxBoundedRawContainer *>(self_ptr);
}

auto gvox_container_bounded_raw_sample(void *self_ptr, void *out_voxel_data, GvoxVoxelDesc out_voxel_desc, GvoxOffset offset) -> GvoxResult {
    auto &self = *static_cast<GvoxBoundedRawContainer *>(self_ptr);
    auto const dim = std::min(offset.axis_n, self.extent.axis_n);

    auto voxel_offset = uint64_t{0};
    auto voxel_size_bytes = (gvox_voxel_desc_size_in_bits(self.voxel_desc) + 7) >> 3;
    auto stride = uint64_t{voxel_size_bytes};

    for (size_t i = 0; i < dim; ++i) {
        if (offset.axis[i] < 0 || offset.axis[i] >= static_cast<int64_t>(self.extent.axis[i])) {
            return GVOX_SUCCESS;
        }
        auto axis_p = static_cast<uint64_t>(offset.axis[i]);
        voxel_offset += axis_p * stride;
        stride *= self.extent.axis[i];
    }

    auto *data = static_cast<uint8_t *>(self.pre_allocated_buffer);
    return gvox_translate_voxel(data + voxel_offset, self.voxel_desc, out_voxel_data, out_voxel_desc);
}

auto gvox_container_bounded_raw_fill(void *self_ptr, void *single_voxel_data, GvoxVoxelDesc src_voxel_desc, GvoxRange range) -> GvoxResult {
    auto &self = *static_cast<GvoxBoundedRawContainer *>(self_ptr);

    // convert src data to be compatible with the dst_voxel_desc
    auto *converted_data = static_cast<void *>(nullptr);
    // test to see if the input data is already compatible (basically if it's the same exact voxel desc)
    auto is_compatible_voxel_desc = [](GvoxVoxelDesc desc_a, GvoxVoxelDesc desc_b) -> bool {
        return desc_a == desc_b;
    };
    if (is_compatible_voxel_desc(src_voxel_desc, self.voxel_desc)) {
        converted_data = single_voxel_data;
    } else {
        // converted_data = convert_data(converted_data_stack);
    }

    if (range.offset.axis_n < range.extent.axis_n) {
        return GVOX_ERROR_INVALID_ARGUMENT;
    }
    auto const dim = std::min(self.extent.axis_n, range.extent.axis_n);

    if (dim < 1) {
        return GVOX_SUCCESS;
    }

    for (uint32_t i = 0; i < dim; ++i) {
        if (range.extent.axis[i] == 0) {
            return GVOX_SUCCESS;
        }
    }

    Voxel in_voxel = {
        .ptr = static_cast<uint8_t *>(converted_data),
        .size = static_cast<uint32_t>((gvox_voxel_desc_size_in_bits(self.voxel_desc) + 7) >> 3),
    };

    auto &offset_buffer = self.pre_alloc_offset_buffer;
    auto voxel_range_offset = GvoxOffsetMut{.axis_n = dim, .axis = offset_buffer.data() + std::ptrdiff_t(0 * dim)};
    auto voxel_range_extent = GvoxExtentMut{.axis_n = dim, .axis = reinterpret_cast<uint64_t *>(offset_buffer.data() + std::ptrdiff_t(1 * dim))};
    auto voxel_next = GvoxOffsetMut{.axis_n = dim, .axis = offset_buffer.data() + std::ptrdiff_t(2 * dim)};

    auto *voxel_ptr = static_cast<uint8_t *>(self.pre_allocated_buffer);
    {
        auto stride = size_t{in_voxel.size};

        bool completely_clipped = false;
        for (uint32_t i = 0; i < dim; ++i) {
            auto p0 = range.offset.axis[i];
            auto p1 = p0 + static_cast<int64_t>(range.extent.axis[i]) - 1;
            if (p0 >= static_cast<int64_t>(self.extent.axis[i]) || p1 < 0) {
                // NOTE: Should be impossible
                completely_clipped = true;
                break;
            }
            if (p0 < 0) {
                p0 = 0;
            }
            if (p1 >= static_cast<int64_t>(self.extent.axis[i])) {
                p1 = static_cast<int64_t>(self.extent.axis[i]) - 1;
            }
            voxel_range_offset.axis[i] = p0;
            voxel_range_extent.axis[i] = static_cast<uint64_t>(p1 - p0 + 1);

            voxel_ptr += static_cast<size_t>(voxel_range_offset.axis[i]) * stride;
            voxel_next.axis[i] = static_cast<int64_t>(static_cast<uint64_t>(stride) * (self.extent.axis[i] - voxel_range_extent.axis[i]));
            stride *= self.extent.axis[i];
        }

        if (completely_clipped) {
            return GVOX_SUCCESS;
        }
    }

    fill_Nd(dim, voxel_ptr, in_voxel, voxel_range_extent, voxel_next);

    return GVOX_SUCCESS;
}