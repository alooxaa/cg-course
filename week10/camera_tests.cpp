#include "camera.hpp"
#include <iostream>
#include <stdexcept>
void check(bool value) { if(!value) throw std::runtime_error("Camera regression"); }
int main() {
    try {
        week10::Camera c;
        c.mouse(1830,970); check(c.yaw==-90 && c.pitch==0);
        c.mouse(2730,970); check(glm::length(c.front-glm::vec3(1,0,0))<0.0001f);
        week10::Input i; i.forward=true; c.move(i,1,3);
        check(glm::length(c.position-glm::vec3(2.5f,0,3))<0.0001f);
        c.mouse(2730,-10000); check(c.pitch==89 && c.front.y>0);
        for(int col=0;col<4;++col) for(int row=0;row<4;++row) check(std::isfinite(c.view()[col][row]));
        c.mouse(2730,10000); check(c.pitch==-89);
        c.scroll(100); check(c.fov==1); c.scroll(-100); check(c.fov==45);
        c.scroll(15); check(c.fov==30);
        c.firstMouse=true; const float yaw=c.yaw; c.mouse(0,0); check(c.yaw==yaw);
        for(int fps : {30,60,144,1000}) {
            c={}; for(int n=0;n<fps;++n) c.move(i,1.0f/fps,3);
            check(std::abs(c.position.z-0.5f)<0.0001f);
        }
        std::cout << "PASS: first mouse, yaw, inverted screen Y, pitch limits, finite view, FOV limits, recapture, WASD and FPS independence\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
