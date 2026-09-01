#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

static GLuint compileShader(GLenum type, const char* source);

struct UiVertex {
    float x;
    float y;
    float r;
    float g;
    float b;
    float a;
};

static const char* glyph(char character) {
    switch (character) {
        case 'A': return "01110 10001 10001 11111 10001 10001 10001";
        case 'B': return "11110 10001 10001 11110 10001 10001 11110";
        case 'C': return "01111 10000 10000 10000 10000 10000 01111";
        case 'D': return "11110 10001 10001 10001 10001 10001 11110";
        case 'E': return "11111 10000 10000 11110 10000 10000 11111";
        case 'F': return "11111 10000 10000 11110 10000 10000 10000";
        case 'G': return "01111 10000 10000 10111 10001 10001 01111";
        case 'H': return "10001 10001 10001 11111 10001 10001 10001";
        case 'I': return "11111 00100 00100 00100 00100 00100 11111";
        case 'K': return "10001 10010 10100 11000 10100 10010 10001";
        case 'L': return "10000 10000 10000 10000 10000 10000 11111";
        case 'M': return "10001 11011 10101 10101 10001 10001 10001";
        case 'N': return "10001 11001 10101 10011 10001 10001 10001";
        case 'O': return "01110 10001 10001 10001 10001 10001 01110";
        case 'P': return "11110 10001 10001 11110 10000 10000 10000";
        case 'R': return "11110 10001 10001 11110 10100 10010 10001";
        case 'S': return "01111 10000 10000 01110 00001 00001 11110";
        case 'T': return "11111 00100 00100 00100 00100 00100 00100";
        case 'U': return "10001 10001 10001 10001 10001 10001 01110";
        case 'V': return "10001 10001 10001 10001 10001 01010 00100";
        case 'Z': return "11111 00001 00010 00100 01000 10000 11111";
        case '0': return "01110 10001 10011 10101 11001 10001 01110";
        case '1': return "00100 01100 00100 00100 00100 00100 01110";
        case '2': return "01110 10001 00001 00010 00100 01000 11111";
        case '3': return "01110 10001 00001 00110 00001 10001 01110";
        case '4': return "00010 00110 01010 10010 11111 00010 00010";
        case '5': return "11111 10000 10000 11110 00001 00001 11110";
        case '6': return "00110 01000 10000 11110 10001 10001 01110";
        case '7': return "11111 00001 00010 00100 01000 01000 01000";
        case '8': return "01110 10001 10001 01110 10001 10001 01110";
        case '9': return "01110 10001 10001 01111 00001 00010 11100";
        case '.': return "00000 00000 00000 00000 00000 00110 00110";
        case '*': return "00000 10101 01110 11111 01110 10101 00000";
        case ':': return "00000 00110 00110 00000 00110 00110 00000";
        case '-': return "00000 00000 00000 11111 00000 00000 00000";
        default: return "00000 00000 00000 00000 00000 00000 00000";
    }
}

static void addUiRect(std::vector<UiVertex>& vertices, float x, float y, float width,
                      float height, float r, float g, float b, float a) {
    UiVertex corners[6] = {
        {x, y, r, g, b, a}, {x + width, y, r, g, b, a},
        {x + width, y + height, r, g, b, a}, {x, y, r, g, b, a},
        {x + width, y + height, r, g, b, a}, {x, y + height, r, g, b, a}
    };
    vertices.insert(vertices.end(), std::begin(corners), std::end(corners));
}

static void addUiText(std::vector<UiVertex>& vertices, const std::string& text,
                      float x, float y, float scale, float r, float g, float b) {
    float cursor = x;
    for (char character : text) {
        const char* pattern = glyph(character);
        int row = 0;
        int column = 0;
        for (const char* pixel = pattern; *pixel; ++pixel) {
            if (*pixel == ' ') {
                ++row;
                column = 0;
            } else if (*pixel == '1') {
                addUiRect(vertices, cursor + column * scale, y + row * scale,
                          scale, scale, r, g, b, 1.0f);
                ++column;
            } else {
                ++column;
            }
        }
        cursor += 6.0f * scale;
    }
}

static GLuint createUiProgram() {
    const char* vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec2 position;
        layout (location = 1) in vec4 vertexColor;
        uniform vec2 resolution;
        out vec4 color;
        void main() {
            vec2 clip = position / resolution * 2.0 - 1.0;
            clip.y = -clip.y;
            gl_Position = vec4(clip, 0.0, 1.0);
            color = vertexColor;
        }
    )";
    const char* fragmentSource = R"(
        #version 330 core
        in vec4 color;
        out vec4 fragment;
        void main() { fragment = color; }
    )";
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!vertexShader || !fragmentShader) return 0;
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<size_t>(length));
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        std::cerr << "Shader compilation failed:\n" << log.data() << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint createProgram() {
    const char* vertexSource = R"(
        #version 330 core
        const vec2 positions[3] = vec2[3](
            vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0)
        );
        out vec2 screenUv;
        void main() {
            vec2 position = positions[gl_VertexID];
            screenUv = position * 0.5 + 0.5;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";

    const char* fragmentSource = R"(
        #version 330 core
        in vec2 screenUv;
        out vec4 color;
        uniform vec2 resolution;
        uniform float time;
        uniform float cameraYaw;
        uniform float cameraPitch;
        uniform float cameraZoom;
        uniform float massScale;
        uniform float spin;
        uniform float diskBrightness;
        uniform float accretionRate;
        uniform float jetPower;

        float hash21(vec2 p) {
            p = fract(p * vec2(123.34, 456.21));
            p += dot(p, p + 45.32);
            return fract(p.x * p.y);
        }

        vec3 starfield(vec2 uv) {
            vec3 stars = vec3(0.0);
            vec2 cellUv = uv * 90.0;
            vec2 cell = floor(cellUv);
            vec2 local = fract(cellUv) - 0.5;
            float star = step(0.985, hash21(cell));
            float size = mix(0.015, 0.09, hash21(cell + 13.7));
            float glow = smoothstep(size, 0.0, length(local));
            vec3 tint = mix(vec3(0.55, 0.72, 1.0), vec3(1.0, 0.72, 0.42), hash21(cell + 4.2));
            stars += tint * star * glow;

            float dust = hash21(floor(uv * 220.0));
            stars += vec3(0.18, 0.25, 0.38) * step(0.997, dust);
            return stars;
        }

        vec3 starsFromDirection(vec3 direction) {
            vec2 skyUv = vec2(atan(direction.z, direction.x), asin(direction.y));
            return starfield(skyUv * vec2(0.75, 1.35));
        }

        vec3 diskEmission(vec3 point, vec3 rayDirection, float distanceTravelled) {
            float diskRadius = length(point.xz);
            float innerRadius = 1.72 * massScale;
            float outerRadius = 5.5 * massScale;
            float diskShape = smoothstep(innerRadius, innerRadius + 0.18, diskRadius)
                            * (1.0 - smoothstep(outerRadius - 0.4, outerRadius, diskRadius));
            float temperature = pow(1.0 - smoothstep(innerRadius, outerRadius, diskRadius), 0.48);
            float normalizedRadius = diskRadius / massScale;
            float angle = atan(point.z, point.x);
            float orbitalRate = (0.34 + 0.10 * spin) / pow(max(normalizedRadius, 1.0), 1.5);
            float fluidPhase = angle - time * orbitalRate * 7.0 + normalizedRadius * 8.0;
            float turbulence = 0.72 + 0.18 * sin(fluidPhase * 5.0)
                             + 0.10 * sin(fluidPhase * 13.0 + point.y * 9.0);

            // Keplerian disk motion: inner material orbits faster than outer material.
            vec3 tangent = normalize(vec3(-point.z, 0.0, point.x));
            float towardObserver = dot(tangent, -rayDirection);
            float orbitalSpeed = clamp((0.34 + 0.10 * spin) * sqrt(innerRadius / diskRadius), 0.06, 0.47);
            float gamma = inversesqrt(1.0 - orbitalSpeed * orbitalSpeed);
            float doppler = 1.0 / (gamma * (1.0 - orbitalSpeed * towardObserver));
            float beaming = pow(doppler, 3.0);

            // Gravitational redshift suppresses energy released close to the horizon.
            float gravitationalRedshift = sqrt(max(0.08, 1.0 - massScale / diskRadius));
            float observedTemperature = temperature * doppler * gravitationalRedshift;
            vec3 orange = vec3(1.0, 0.16, 0.015);
            vec3 whiteHot = vec3(1.0, 0.93, 0.62);
            vec3 blueHot = vec3(0.34, 0.62, 1.0);
            vec3 redShifted = vec3(0.72, 0.035, 0.008);
            vec3 heat = mix(redShifted, orange, smoothstep(0.15, 0.60, observedTemperature));
            heat = mix(heat, whiteHot, smoothstep(0.60, 1.0, observedTemperature));
            heat = mix(heat, blueHot, smoothstep(1.0, 1.35, observedTemperature));
            float attenuation = 1.0 / (1.0 + distanceTravelled * 0.035);
            float luminosity = 0.32 + 0.68 * sqrt(clamp(accretionRate, 0.0, 1.0));
              return heat * diskShape * turbulence * beaming * gravitationalRedshift
                  * attenuation * diskBrightness * luminosity;
        }

        float diskDensity(vec3 point) {
            float normalizedRadius = length(point.xz) / massScale;
            float radialBand = smoothstep(1.65, 1.95, normalizedRadius)
                             * (1.0 - smoothstep(5.0, 5.8, normalizedRadius));
            float thickness = mix(0.10, 0.24, smoothstep(1.65, 5.8, normalizedRadius));
            float angle = atan(point.z, point.x);
            float orbitalRate = (0.34 + 0.10 * spin) / pow(max(normalizedRadius, 1.0), 1.5);
            float fluidPhase = angle - time * orbitalRate * 7.0 + normalizedRadius * 8.0;
            float fluidClumps = 0.72 + 0.28 * sin(fluidPhase * 5.0)
                              + 0.10 * sin(fluidPhase * 13.0 + time);
            return radialBand * max(fluidClumps, 0.18)
                 * exp(-abs(point.y) / (massScale * thickness));
        }

        float jetDensity(vec3 point) {
            float axialDistance = abs(point.y) / massScale;
            float radialDistance = length(point.xz) / massScale;
            float activeRegion = smoothstep(0.9, 1.2, axialDistance)
                                * (1.0 - smoothstep(11.0, 13.0, axialDistance));
            float coneRadius = 0.10 + axialDistance * 0.055;
            float edgeDistance = abs(radialDistance - coneRadius);
            float core = exp(-edgeDistance * edgeDistance / 0.018);
            float innerBeam = exp(-radialDistance * radialDistance / 0.018);
            return activeRegion * max(core, innerBeam * 0.35);
        }

        vec3 jetEmission(vec3 point, vec3 rayDirection, float distanceTravelled) {
            float density = jetDensity(point);
            float axialDirection = sign(point.y);
            vec3 jetDirection = vec3(0.0, axialDirection, 0.0);
            float beaming = 1.0 + 1.8 * max(dot(jetDirection, -rayDirection), 0.0);
            float turbulence = 0.72 + 0.28 * sin(point.y * 15.0 + point.x * 22.0 + time * 3.0);
            float distanceFade = 1.0 / (1.0 + distanceTravelled * 0.025);
            vec3 jetColor = mix(vec3(0.03, 0.20, 1.0), vec3(0.78, 0.94, 1.0),
                                smoothstep(0.25, 0.9, turbulence));
            float powerScale = 0.28 + 0.72 * sqrt(clamp(jetPower, 0.0, 1.0));
              return jetColor * density * beaming * turbulence * distanceFade
                  * diskBrightness * powerScale * 0.52;
        }

        vec3 rayAcceleration(vec3 position, vec3 direction) {
            float distanceFromCenter = max(length(position), massScale);
            vec3 radial = position / distanceFromCenter;
            float radialSpeed = dot(direction, radial);
            // Bend only the transverse part of the ray toward the center.
            // A perfectly radial ray should continue straight inward, not be pushed outward.
            vec3 gravity = -1.5 * massScale * (radial - radialSpeed * direction)
                          / (distanceFromCenter * distanceFromCenter);
            vec3 frameDragging = spin * massScale * cross(vec3(0.0, 1.0, 0.0), direction)
                               / (distanceFromCenter * distanceFromCenter * distanceFromCenter);
            return gravity + frameDragging;
        }

        // Kerr null-geodesic potentials in geometric units (G = c = M = 1).
        float kerrRadialPotential(float radius, float a, float xi, float eta) {
            float delta = radius * radius - 2.0 * radius + a * a;
            float p = radius * radius + a * a - a * xi;
            return p * p - delta * ((xi - a) * (xi - a) + eta);
        }

        float kerrPolarPotential(float theta, float a, float xi, float eta) {
            float sine = max(sin(theta), 0.001);
            float cosine = cos(theta);
            return eta + a * a * cosine * cosine
                 - xi * xi * cosine * cosine / (sine * sine);
        }

        void integrateRayRK4(inout vec3 position, inout vec3 direction, float stepSize) {
            vec3 positionK1 = direction;
            vec3 directionK1 = rayAcceleration(position, direction);

            vec3 position2 = position + positionK1 * stepSize * 0.5;
            vec3 direction2 = normalize(direction + directionK1 * stepSize * 0.5);
            vec3 positionK2 = direction2;
            vec3 directionK2 = rayAcceleration(position2, direction2);

            vec3 position3 = position + positionK2 * stepSize * 0.5;
            vec3 direction3 = normalize(direction + directionK2 * stepSize * 0.5);
            vec3 positionK3 = direction3;
            vec3 directionK3 = rayAcceleration(position3, direction3);

            vec3 position4 = position + positionK3 * stepSize;
            vec3 direction4 = normalize(direction + directionK3 * stepSize);
            vec3 positionK4 = direction4;
            vec3 directionK4 = rayAcceleration(position4, direction4);

            position += stepSize * (positionK1 + 2.0 * positionK2
                                  + 2.0 * positionK3 + positionK4) / 6.0;
            direction = normalize(direction + stepSize * (directionK1 + 2.0 * directionK2
                                  + 2.0 * directionK3 + directionK4) / 6.0);
        }

        void main() {
            vec2 uv = (gl_FragCoord.xy - 0.5 * resolution) / resolution.y;
            uv *= cameraZoom;
            float cameraDistance = 8.0 * massScale;
            vec3 cameraPosition = vec3(
                cameraDistance * cos(cameraPitch) * sin(cameraYaw),
                cameraDistance * sin(cameraPitch),
                cameraDistance * cos(cameraPitch) * cos(cameraYaw)
            );
            vec3 accumulated = vec3(0.0);
            float transmittance = 1.0;
            float closestApproach = 100.0;
            bool captured = false;

            // Trace a null geodesic using the separated Kerr equations.
            float observerTheta = 1.5707963 - cameraPitch;
            float alpha = uv.x * 6.0;
            float beta = uv.y * 6.0;
            float sineObserver = max(sin(observerTheta), 0.001);
            float xi = -alpha * sineObserver;
            float eta = beta * beta + cos(observerTheta) * cos(observerTheta)
                      * (alpha * alpha - spin * spin);
            float horizon = 1.0 + sqrt(max(0.0, 1.0 - spin * spin));
            float radius = 24.0;
            float theta = observerTheta;
            float phi = cameraYaw;
            float previousTheta = theta;
            vec3 previousPoint = vec3(radius * sin(theta), radius * cos(theta), 0.0);
            vec3 rayDirection = normalize(-previousPoint);
            float travel = 0.0;

            for (int step = 0; step < 260; ++step) {
                float delta = radius * radius - 2.0 * radius + spin * spin;
                float sigma = radius * radius + spin * spin * cos(theta) * cos(theta);
                float radialPotential = max(kerrRadialPotential(radius, spin, xi, eta), 0.0);
                float polarPotential = max(kerrPolarPotential(theta, spin, xi, eta), 0.0);
                float radialVelocity = -sqrt(radialPotential) / max(sigma, 0.001);
                // Rays are traced backward from the observer, so screen beta has inverted theta sign.
                float polarVelocity = (beta < 0.0 ? 1.0 : -1.0)
                                    * sqrt(polarPotential) / max(sigma, 0.001);
                float phiVelocity = (spin * (radius * radius + spin * spin - spin * xi)
                                   / max(delta, 0.0001)
                                   - (spin - xi / max(sin(theta) * sin(theta), 0.001)))
                                   / max(sigma, 0.001);
                float affineStep = mix(0.035, 0.12, smoothstep(horizon + 0.1, 6.0, radius));
                radius += radialVelocity * affineStep;
                theta += polarVelocity * affineStep;
                phi += phiVelocity * affineStep;
                travel += affineStep;

                closestApproach = min(closestApproach, radius);
                if (radius <= horizon) {
                    captured = true;
                    break;
                }
                if (radius > 24.0 || theta <= 0.02 || theta >= 3.12) break;

                vec3 point = vec3(radius * sin(theta) * cos(phi),
                                  radius * cos(theta),
                                  radius * sin(theta) * sin(phi));
                vec3 segmentDirection = normalize(point - previousPoint);
                float stepSize = length(point - previousPoint);
                float faintDiskAtmosphere = diskDensity(point);
                float diskRadiusAlongRay = length(point.xz) / massScale;
                vec3 faintDiskColor = mix(vec3(0.34, 0.018, 0.002),
                                          vec3(0.95, 0.24, 0.015),
                                          1.0 - smoothstep(1.65, 5.8, diskRadiusAlongRay));
                accumulated += faintDiskColor * faintDiskAtmosphere * stepSize
                             * transmittance * 0.055;
                // Optical depth: dense plasma absorbs light along the ray.
                transmittance *= exp(-faintDiskAtmosphere * stepSize * 0.48);
                float jetAtmosphere = jetDensity(point);
                accumulated += jetEmission(point, segmentDirection, travel)
                             * transmittance * stepSize;
                transmittance *= exp(-jetAtmosphere * stepSize * 0.18);
                if ((previousTheta > 1.5707963 && theta <= 1.5707963)
                    || (previousTheta < 1.5707963 && theta >= 1.5707963)) {
                    float crossingFactor = (1.5707963 - previousTheta)
                                         / max(theta - previousTheta, 0.0001);
                    vec3 crossing = previousPoint + (point - previousPoint) * crossingFactor;
                    accumulated += diskEmission(crossing, rayDirection, travel) * transmittance;
                    transmittance *= 0.72;
                }
                previousTheta = theta;
                previousPoint = point;
                rayDirection = segmentDirection;
            }

            // Only a thin, dim warm photon ring should surround the black shadow.
            float photonRing = exp(-pow((closestApproach - 1.52) / 0.012, 2.0));
            if (!captured) {
                vec3 lensedStars = starsFromDirection(rayDirection);
                float lensingMagnification = 1.0 + 0.85 * smoothstep(2.8, 1.52, closestApproach);
                accumulated += lensedStars * transmittance
                             * (0.42 + photonRing * 1.15) * lensingMagnification;
                accumulated += vec3(1.0, 0.20, 0.025) * photonRing * 0.075;
            }
            color = vec4(accumulated, 1.0);
        }
    )";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!vertexShader || !fragmentShader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<size_t>(length));
        glGetProgramInfoLog(program, length, nullptr, log.data());
        std::cerr << "Program linking failed:\n" << log.data() << std::endl;
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

static GLuint createPostProgram() {
    const char* vertexSource = R"(
        #version 330 core
        const vec2 positions[3] = vec2[3](
            vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0)
        );
        out vec2 screenUv;
        void main() {
            vec2 position = positions[gl_VertexID];
            screenUv = position * 0.5 + 0.5;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";
    const char* fragmentSource = R"(
        #version 330 core
        in vec2 screenUv;
        out vec4 color;
        uniform sampler2D inputTexture;
        uniform sampler2D hdrScene;
        uniform vec2 resolution;
        uniform vec2 direction;
        uniform bool combineScene;

        vec3 acesToneMap(vec3 value) {
            value = max(value, vec3(0.0));
            return clamp((value * (2.51 * value + 0.03))
                       / (value * (2.43 * value + 0.59) + 0.14), 0.0, 1.0);
        }

        void main() {
            vec2 pixel = 1.0 / vec2(textureSize(inputTexture, 0));
            vec3 blurred = vec3(0.0);
            blurred += texture(inputTexture, screenUv).rgb * 0.227027;
            blurred += texture(inputTexture, screenUv + direction * pixel * 1.384615).rgb * 0.316216;
            blurred += texture(inputTexture, screenUv - direction * pixel * 1.384615).rgb * 0.316216;
            blurred += texture(inputTexture, screenUv + direction * pixel * 3.230769).rgb * 0.070270;
            blurred += texture(inputTexture, screenUv - direction * pixel * 3.230769).rgb * 0.070270;

            if (!combineScene) {
                color = vec4(max(blurred - vec3(1.0), vec3(0.0)), 1.0);
                return;
            }

            vec3 hdr = texture(hdrScene, screenUv).rgb;
            vec3 mapped = acesToneMap(hdr * 1.65 + blurred * 2.10);
            mapped = pow(mapped, vec3(1.0 / 2.2));
            color = vec4(mapped, 1.0);
        }
    )";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!vertexShader || !fragmentShader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<size_t>(length));
        glGetProgramInfoLog(program, length, nullptr, log.data());
        std::cerr << "Post-process program linking failed:\n" << log.data() << std::endl;
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1100, 700, "Black Hole | Schwarzschild View", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    GLuint fullscreenVao = 0;
    glGenVertexArrays(1, &fullscreenVao);
    glBindVertexArray(fullscreenVao);

    GLuint program = createProgram();
    if (!program) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    GLuint postProgram = createPostProgram();
    if (!postProgram) {
        glDeleteProgram(program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    GLuint uiProgram = createUiProgram();
    if (!uiProgram) {
        glDeleteProgram(postProgram);
        glDeleteProgram(program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    GLuint uiVao = 0;
    GLuint uiVbo = 0;
    glGenVertexArrays(1, &uiVao);
    glGenBuffers(1, &uiVbo);

    GLuint hdrFramebuffer = 0;
    GLuint hdrTexture = 0;
    GLuint bloomFramebuffer = 0;
    GLuint bloomTexture = 0;
    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    int renderWidth = std::max(1, framebufferWidth * 3 / 4);
    int renderHeight = std::max(1, framebufferHeight * 3 / 4);
    glGenFramebuffers(1, &hdrFramebuffer);
    glGenTextures(1, &hdrTexture);
    glGenFramebuffers(1, &bloomFramebuffer);
    glGenTextures(1, &bloomTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderWidth, renderHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTexture, 0);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderWidth, renderHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "HDR framebuffer is incomplete" << std::endl;
        glDeleteFramebuffers(1, &hdrFramebuffer);
        glDeleteTextures(1, &hdrTexture);
        glDeleteFramebuffers(1, &bloomFramebuffer);
        glDeleteTextures(1, &bloomTexture);
        glDeleteProgram(postProgram);
        glDeleteProgram(program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
        glViewport(0, 0, width, height);
    });

    struct CameraInput {
        bool dragging = false;
        double lastX = 0.0;
        double lastY = 0.0;
        float yaw = 0.0f;
        float pitch = 0.78f;
        float zoom = 1.0f;
        float massScale = 1.0f;
        float spin = 0.12f;
        float diskBrightness = 0.68f;
        float accretionRate = 0.000001f;
        float jetPower = 0.00001f;
        std::string massLabel = "4.3E6";
        std::string presetName = "Sagittarius A*";
    } camera;
    glfwSetWindowUserPointer(window, &camera);
    glfwSetMouseButtonCallback(window, [](GLFWwindow* currentWindow, int button, int action, int) {
        CameraInput* input = static_cast<CameraInput*>(glfwGetWindowUserPointer(currentWindow));
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            input->dragging = action == GLFW_PRESS;
            if (input->dragging) glfwGetCursorPos(currentWindow, &input->lastX, &input->lastY);
        }
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* currentWindow, double x, double y) {
        CameraInput* input = static_cast<CameraInput*>(glfwGetWindowUserPointer(currentWindow));
        if (!input->dragging) return;
        input->yaw += static_cast<float>(x - input->lastX) * 0.008f;
        input->pitch -= static_cast<float>(y - input->lastY) * 0.008f;
        input->pitch = std::max(0.12f, std::min(1.52f, input->pitch));
        input->lastX = x;
        input->lastY = y;
    });
    glfwSetScrollCallback(window, [](GLFWwindow* currentWindow, double, double yOffset) {
        CameraInput* input = static_cast<CameraInput*>(glfwGetWindowUserPointer(currentWindow));
        input->zoom -= static_cast<float>(yOffset) * 0.08f;
        input->zoom = std::max(0.58f, std::min(1.65f, input->zoom));
    });
    glfwSetKeyCallback(window, [](GLFWwindow* currentWindow, int key, int, int action, int) {
        if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
        CameraInput* input = static_cast<CameraInput*>(glfwGetWindowUserPointer(currentWindow));
        if (key == GLFW_KEY_1) {
            input->presetName = "Sagittarius A*";
            input->massScale = 1.0f;
            input->spin = 0.12f;
            input->diskBrightness = 0.68f;
            input->accretionRate = 0.000001f;
            input->jetPower = 0.00001f;
            input->massLabel = "4.3E6";
        } else if (key == GLFW_KEY_2) {
            input->presetName = "TON 618";
            input->massScale = 1.8f;
            input->spin = 0.72f;
            input->diskBrightness = 1.15f;
            input->accretionRate = 0.30f;
            input->jetPower = 0.25f;
            input->massLabel = "6.6E10";
        } else if (key == GLFW_KEY_LEFT_BRACKET) {
            input->massScale = std::max(0.55f, input->massScale - 0.05f);
        } else if (key == GLFW_KEY_RIGHT_BRACKET) {
            input->massScale = std::min(2.5f, input->massScale + 0.05f);
        } else if (key == GLFW_KEY_COMMA) {
            input->spin = std::max(0.0f, input->spin - 0.03f);
        } else if (key == GLFW_KEY_PERIOD) {
            input->spin = std::min(0.998f, input->spin + 0.03f);
        } else if (key == GLFW_KEY_MINUS) {
            input->diskBrightness = std::max(0.15f, input->diskBrightness - 0.05f);
        } else if (key == GLFW_KEY_EQUAL) {
            input->diskBrightness = std::min(2.0f, input->diskBrightness + 0.05f);
        } else if (key == GLFW_KEY_R) {
            input->yaw = 0.0f;
            input->pitch = 0.78f;
            input->zoom = 1.0f;
        }
    });

    double fpsStartTime = glfwGetTime();
    double previousFrameTime = fpsStartTime;
    double displayedFps = 0.0;
    glUseProgram(program);

    while (!glfwWindowShouldClose(window)) {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        int desiredRenderWidth = std::max(1, width * 3 / 4);
        int desiredRenderHeight = std::max(1, height * 3 / 4);
        if (desiredRenderWidth != renderWidth || desiredRenderHeight != renderHeight) {
            renderWidth = desiredRenderWidth;
            renderHeight = desiredRenderHeight;
            framebufferWidth = width;
            framebufferHeight = height;
            glBindTexture(GL_TEXTURE_2D, hdrTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderWidth, renderHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
            glBindTexture(GL_TEXTURE_2D, bloomTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderWidth, renderHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFramebuffer);
        glViewport(0, 0, renderWidth, renderHeight);
        glClearColor(0.002f, 0.003f, 0.008f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glUniform2f(glGetUniformLocation(program, "resolution"),
                    static_cast<float>(renderWidth), static_cast<float>(renderHeight));
        glUniform1f(glGetUniformLocation(program, "time"), static_cast<float>(glfwGetTime()));
        glUniform1f(glGetUniformLocation(program, "cameraYaw"), camera.yaw);
        glUniform1f(glGetUniformLocation(program, "cameraPitch"), camera.pitch);
        glUniform1f(glGetUniformLocation(program, "cameraZoom"), camera.zoom);
        glUniform1f(glGetUniformLocation(program, "massScale"), camera.massScale);
        glUniform1f(glGetUniformLocation(program, "spin"), camera.spin);
        glUniform1f(glGetUniformLocation(program, "diskBrightness"), camera.diskBrightness);
        glUniform1f(glGetUniformLocation(program, "accretionRate"), camera.accretionRate);
        glUniform1f(glGetUniformLocation(program, "jetPower"), camera.jetPower);
        glBindVertexArray(fullscreenVao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindFramebuffer(GL_FRAMEBUFFER, bloomFramebuffer);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(postProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glUniform1i(glGetUniformLocation(postProgram, "inputTexture"), 0);
        glUniform2f(glGetUniformLocation(postProgram, "resolution"),
            static_cast<float>(renderWidth), static_cast<float>(renderHeight));
        glUniform2f(glGetUniformLocation(postProgram, "direction"), 1.0f, 0.0f);
        glUniform1i(glGetUniformLocation(postProgram, "combineScene"), GL_FALSE);
        glBindVertexArray(fullscreenVao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(postProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bloomTexture);
        glUniform1i(glGetUniformLocation(postProgram, "inputTexture"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glUniform1i(glGetUniformLocation(postProgram, "hdrScene"), 1);
        glUniform2f(glGetUniformLocation(postProgram, "resolution"),
                static_cast<float>(width), static_cast<float>(height));
        glUniform2f(glGetUniformLocation(postProgram, "direction"), 0.0f, 1.0f);
        glUniform1i(glGetUniformLocation(postProgram, "combineScene"), GL_TRUE);
        glBindVertexArray(fullscreenVao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        std::vector<UiVertex> uiVertices;
        addUiRect(uiVertices, static_cast<float>(width - 220), 24.0f, 196.0f, 42.0f,
              0.015f, 0.025f, 0.055f, 0.88f);
        addUiRect(uiVertices, static_cast<float>(width - 220), 24.0f, 196.0f, 3.0f,
              0.20f, 0.95f, 0.62f, 1.0f);
        addUiText(uiVertices, "FPS " + std::to_string(static_cast<int>(displayedFps)),
              static_cast<float>(width - 204), 39.0f, 2.5f, 0.35f, 1.0f, 0.58f);
        float fpsFill = static_cast<float>(std::min(displayedFps / 60.0, 1.0));
        addUiRect(uiVertices, static_cast<float>(width - 204), 60.0f, 160.0f, 4.0f,
              0.07f, 0.10f, 0.16f, 1.0f);
        addUiRect(uiVertices, static_cast<float>(width - 204), 60.0f, 160.0f * fpsFill, 4.0f,
              fpsFill > 0.5f ? 0.20f : 1.0f, fpsFill > 0.5f ? 0.95f : 0.38f,
              0.28f, 1.0f);
        addUiRect(uiVertices, 24.0f, 24.0f, 310.0f, 270.0f, 0.015f, 0.025f, 0.055f, 0.88f);
        addUiRect(uiVertices, 24.0f, 24.0f, 270.0f, 3.0f, 0.95f, 0.28f, 0.06f, 1.0f);
        addUiText(uiVertices, "BLACK HOLE LAB", 40.0f, 44.0f, 3.0f, 0.88f, 0.92f, 1.0f);
        addUiText(uiVertices, "1 SAGITTARIUS A*", 40.0f, 70.0f, 2.5f, 0.52f, 0.72f, 0.95f);
        addUiText(uiVertices, "2 TON 618", 40.0f, 90.0f, 2.5f, 0.52f, 0.72f, 0.95f);
        addUiText(uiVertices, "MASS", 40.0f, 120.0f, 2.5f, 0.65f, 0.68f, 0.76f);
        addUiText(uiVertices, camera.massLabel, 155.0f, 120.0f, 2.5f, 1.0f, 0.62f, 0.22f);
        addUiText(uiVertices, "SCALE", 40.0f, 142.0f, 2.5f, 0.65f, 0.68f, 0.76f);
        addUiText(uiVertices, std::to_string(camera.massScale).substr(0, 4), 155.0f, 142.0f, 2.5f, 0.95f, 0.70f, 0.30f);
        addUiText(uiVertices, "SPIN", 40.0f, 164.0f, 2.5f, 0.65f, 0.68f, 0.76f);
        addUiText(uiVertices, std::to_string(camera.spin).substr(0, 4), 155.0f, 164.0f, 2.5f, 0.35f, 0.72f, 1.0f);
        addUiText(uiVertices, "DISK", 40.0f, 186.0f, 2.5f, 0.65f, 0.68f, 0.76f);
        addUiText(uiVertices, std::to_string(camera.diskBrightness).substr(0, 4), 155.0f, 186.0f, 2.5f, 1.0f, 0.42f, 0.12f);
        addUiText(uiVertices, "FPS", 40.0f, 208.0f, 2.5f, 0.65f, 0.68f, 0.76f);
        addUiText(uiVertices, std::to_string(static_cast<int>(displayedFps)), 155.0f, 208.0f, 2.5f, 0.35f, 1.0f, 0.58f);
        addUiText(uiVertices, "DRAG ORBIT  SCROLL ZOOM  R RESET", 40.0f, 240.0f, 1.7f, 0.58f, 0.62f, 0.72f);
        glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
        glBufferData(GL_ARRAY_BUFFER, uiVertices.size() * sizeof(UiVertex), uiVertices.data(), GL_STREAM_DRAW);
        glUseProgram(uiProgram);
        glUniform2f(glGetUniformLocation(uiProgram, "resolution"), static_cast<float>(width), static_cast<float>(height));
        glBindVertexArray(uiVao);
        glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UiVertex), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(UiVertex), reinterpret_cast<void*>(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(uiVertices.size()));
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        glfwPollEvents();
        double currentFrameTime = glfwGetTime();
        double frameSeconds = currentFrameTime - previousFrameTime;
        previousFrameTime = currentFrameTime;
        if (frameSeconds > 0.0) {
            double instantFps = 1.0 / frameSeconds;
            displayedFps = displayedFps == 0.0
                ? instantFps
                : displayedFps * 0.90 + instantFps * 0.10;
        }
        double fpsElapsed = currentFrameTime - fpsStartTime;
        if (fpsElapsed >= 0.5) {
            glfwSetWindowTitle(window, "Black Hole Lab");
            fpsStartTime = currentFrameTime;
        }
    }

    glDeleteProgram(program);
    glDeleteProgram(postProgram);
    glDeleteProgram(uiProgram);
    glDeleteFramebuffers(1, &hdrFramebuffer);
    glDeleteTextures(1, &hdrTexture);
    glDeleteFramebuffers(1, &bloomFramebuffer);
    glDeleteTextures(1, &bloomTexture);
    glDeleteVertexArrays(1, &fullscreenVao);
    glDeleteBuffers(1, &uiVbo);
    glDeleteVertexArrays(1, &uiVao);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
