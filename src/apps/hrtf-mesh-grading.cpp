// Copyright 2011-2021 the Polygon Mesh Processing Library developers.
// Distributed under a MIT-style license, see LICENSE.txt for details.

#include <pmp/algorithms/SurfaceRemeshing.h>
#include <pmp/SurfaceMesh.h>
#include <unistd.h>

using namespace pmp;

void usage_and_exit()
{
    std::cerr << "Usage:\nhrtf-mesh-grading -x <minimum edge lenght> -y <maximum edge length> -e <error> -z <\"left\" or \"right\"> -i <input> -o <output>\n\nOptions\n"
              << "edge lengths should be given in mm"
              << "\n";
    exit(1);
}

int main(int argc, char** argv)
{
    bool binary = false;
    const char* input = nullptr;
    const char* output = nullptr;
    float min, max, err;
    char* ear = "none";

    // parse command line parameters
    int c;
    while ((c = getopt(argc, argv, "x:y:e:z:i:o:")) != -1)
    {
        switch (c)
        {
            case 'x':
                min = std::stof(optarg);
                break;

            case 'y':
                max = std::stof(optarg);
                break;

            case 'e':
                err = std::stof(optarg);
                break;

            case 'z':
                ear = optarg;
                break;

            case 'i':
                input = optarg;
                break;

            case 'o':
                output = optarg;
                break;

            default:
                usage_and_exit();
        }
    }

    // we need input and output mesh
    if (!input || !output)
    {
        usage_and_exit();
    }

    // load input mesh
    SurfaceMesh mesh;
    try
    {
        mesh.read(input);
    }
    catch (const IOException& e)
    {
        std::cerr << "Failed to read mesh: " << e.what() << std::endl;
        exit(1);
    }

    SurfaceRemeshing(mesh).adaptive_remeshing(
                    min,  // min length
                    max,    // max length
                    err,  // approx. error
                    10U,
                    true,
                    ear
                    ); 

    // write output mesh
    IOFlags flags;
    flags.use_binary = binary;
    try
    {
        mesh.write(output, flags);
    }
    catch (const IOException& e)
    {
        std::cerr << "Failed to write mesh: " << e.what() << std::endl;
        exit(1);
    }

    exit(0);
}