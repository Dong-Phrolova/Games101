//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
#include <thread>
#include <vector>
#include <atomic>

void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);

    int spp = 64;

    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> workers;

    std::atomic<int> completed_rows(0);

    std::cout << "Rendering with " << num_threads << " threads, SPP: " << spp << "\n";

    auto render_task = [&](int start_row, int end_row) {
        for (uint32_t j = start_row; j < end_row; ++j) {
            for (uint32_t i = 0; i < scene.width; ++i) {

                int index = j * scene.width + i;

                float x = (2 * (i + 0.5) / (float)scene.width - 1) * imageAspectRatio * scale;
                float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;
                Vector3f dir = normalize(Vector3f(-x, y, 1));

                for (int k = 0; k < spp; k++) {
                    framebuffer[index] += scene.castRay(Ray(eye_pos, dir), 0) / spp;
                }
            }
            completed_rows++;
            UpdateProgress(completed_rows / (float)scene.height);
        }
        };

    int rows_per_thread = scene.height / num_threads;
    for (int t = 0; t < num_threads; ++t) {
        int start = t * rows_per_thread;
        int end = (t == num_threads - 1) ? scene.height : (t + 1) * rows_per_thread;

        workers.push_back(std::thread(render_task, start, end));
    }


    for (auto& worker : workers) {
        worker.join();
    }
    UpdateProgress(1.f);
    
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}
