#version 150

uniform bool uAntialiasing;
uniform int uColors;
uniform int uSteps;
uniform int uThreshold;
uniform int uWindowY;
uniform float uZoom;

in vec2 vPosition;

out vec4 fragColor;

vec2 complex_add(vec2 a, vec2 b) {
    return vec2(
        a.x + b.x,
        a.y + b.y
    );
}

vec2 complex_mul(vec2 a, vec2 b) {
    return vec2(
        a.x * b.x - a.y * b.y,
        a.x * b.y + a.y * b.x
    );
}

float complex_norm_squared(vec2 c) {
    return c.x * c.x + c.y * c.y;
}

vec2 step(vec2 c, vec2 z) {
    return complex_add(c, complex_mul(z, z));
}

int steps(vec2 c) {
    vec2 z = vec2(0, 0);
    for (int i = 1; i <= uSteps; i++) {
        z = step(c, z);
        if (complex_norm_squared(z) > uThreshold * uThreshold) {
            return i;
        }
    }
    return -1;
}

vec4 color(vec2 c) {
    int steps = steps(c);
    if (steps == -1) {
        return vec4(0, 0, 0, 0); // BLACK:      (0, 0, 0)
    }
    float color = 6.0 * ((steps - 1) % uColors) / uColors;
    if (color <= 1) {            // RED:        (1, 0, 0)
        return vec4(1, color, 0, 0);
    } else if (color <= 2) {     // YELLOW:     (1, 1, 0)
        return vec4(2 - color, 1, 0, 0);
    } else if (color <= 3) {     // GREEN:      (0, 1, 0)
        return vec4(0, 1, color - 2, 0);
    } else if (color <= 4) {     // AQUAMARINE: (0, 1, 1)
        return vec4(0, 4 - color, 1, 0);
    } else if (color <= 5) {     // BLUE:       (0, 0, 1)
        return vec4(color - 4, 0, 1, 0);
    } else {                     // PURPLE:     (1, 0, 1)
        return vec4(1, 0, 4 - color, 0);
    }                            // RED:        (1, 0, 0)
}

void main(void) {
    if (!uAntialiasing) {
        fragColor = color(vPosition);
    } else {
        float pixel = 1 / (uWindowY * pow(2, uZoom));
        int samples = 5;
        vec4 sample1 = color(vPosition);
        vec4 sample2 = color(vPosition + vec2(-pixel, -pixel));
        vec4 sample3 = color(vPosition + vec2(-pixel,  pixel));
        vec4 sample4 = color(vPosition + vec2( pixel, -pixel));
        vec4 sample5 = color(vPosition + vec2( pixel,  pixel));
        fragColor = (sample1 + sample2 + sample3 + sample4 + sample5) / samples;
    }
}
