#ifndef GVOX_COLORED_TEXT_CONTAINER_H
#define GVOX_COLORED_TEXT_CONTAINER_H

typedef struct {
    // Doesn't make sense to have a default for this, when the default channel is color data
    uint32_t non_color_max_value;
    // By default, this should be 0
    // if set to 1, each layer will be printed below the last
    uint8_t vertical;
} GvoxColoredTextContainerConfig;

#endif
