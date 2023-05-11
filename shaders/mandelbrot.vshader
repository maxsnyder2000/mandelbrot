#version 150

uniform float uCenterX;
uniform float uCenterY;
uniform int uWindowX;
uniform int uWindowY;
uniform float uZoom;

in vec2 aPosition;

out vec2 vPosition;

void main() {
    float zoomX = pow(2, uZoom) * uWindowY / uWindowX;
    float zoomY = pow(2, uZoom);
    gl_Position = vec4(aPosition.x, aPosition.y, 0, 1);
    vPosition = vec2(aPosition.x / zoomX + uCenterX, aPosition.y / zoomY + uCenterY);
}
