// Copyright 2011-2021 the Polygon Mesh Processing Library developers.
// Distributed under a MIT-style license, see LICENSE.txt for details.

#include <pmp/algorithms/SurfaceRemeshing.h>
#include <pmp/SurfaceMesh.h>
#include <unistd.h>

using namespace pmp;

void usage_and_exit()
{
    std::cerr << "\nExample usage\n-------------\n"
              << "hrtf-mesh-grading -x 0.5 -y 10 -s 'left' -i head.ply -o head_left.ply\n\n"
              << "Parameters\n----------\n"
              << "-x the minimum edge length in mm\n"
              << "-y the maximum edge length in mm\n"
              << "-e the maximum remeshing in mm (Optional. The minimum edge length by default)\n"
              << "-s the side at which the mesh resolution will be high ('left' or 'right')\n"
              << "-i the path to the input mesh\n"
              << "-o the path to the output mesh\n\n"
              << "Note\n----\n"
              << "The interaural center of the head-mesh must be at the origin of coordinates and the mesh must view in positive x-direction.\n\n"
              << "Reference\n---------\n"
              << "T. Palm, S. Koch, F. Brinkmann, and M. Alexa, “Curvature-adaptive mesh grading for numerical approximation of head-related transfer functions,” in DAGA 2021, Vienna, Austria, pp. 1111-1114.\n\n";

    exit(1);
}

int main(int argc, char** argv)
{
    bool binary = false;
    const char* input = nullptr;
    const char* output = nullptr;
    float min, max, err;
    const char* ear = nullptr;

    // parse command line parameters
    int c;
    while ((c = getopt(argc, argv, "x:y:e:s:i:o:")) != -1)
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

            case 's':
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