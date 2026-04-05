__kernel void grayscale(
    __read_only image2d_t input,
    __write_only image2d_t output
) {
    const int2 pos = (int2)(get_global_id(0), get_global_id(1));
    const int2 imgSize = get_image_dim(input);

    // Bounds check - return if outside image dimensions
    if (pos.x >= imgSize.x || pos.y >= imgSize.y) {
        return;
    }

    // Sampler for reading input image
    // CLK_NORMALIZED_COORDS_FALSE: use pixel coordinates directly
    // CLK_ADDRESS_CLAMP: clamp to edge for out-of-bounds reads
    // CLK_FILTER_NEAREST: nearest neighbor sampling (no interpolation)
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
                              CLK_ADDRESS_CLAMP |
                              CLK_FILTER_NEAREST;

    // Read pixel as float4 (RGBA)
    float4 pixel = read_imagef(input, sampler, pos);

    // Convert to grayscale using luminance formula
    // Standard weights: 0.299*R + 0.587*G + 0.114*B
    float gray = 0.299f * pixel.x + 0.587f * pixel.y + 0.114f * pixel.z;

    // Write grayscale result (keep alpha channel at 1.0)
    float4 result = (float4)(gray, gray, gray, 1.0f);
    write_imagef(output, pos, result);
}

__kernel void invert(
    __read_only image2d_t input,
    __write_only image2d_t output
) {
    const int2 pos = (int2)(get_global_id(0), get_global_id(1));
    const int2 imgSize = get_image_dim(input);

    // Bounds check
    if (pos.x >= imgSize.x || pos.y >= imgSize.y) {
        return;
    }

    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
                              CLK_ADDRESS_CLAMP |
                              CLK_FILTER_NEAREST;

    // Read pixel
    float4 pixel = read_imagef(input, sampler, pos);

    // Invert colors (1.0 - channel), keep alpha
    float4 result = (float4)(
        1.0f - pixel.x,
        1.0f - pixel.y,
        1.0f - pixel.z,
        pixel.w
    );

    write_imagef(output, pos, result);
}
