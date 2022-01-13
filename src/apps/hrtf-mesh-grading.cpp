// Copyright 2011-2021 the Polygon Mesh Processing Library developers.
// Distributed under a MIT-style license, see LICENSE.txt for details.

#include <pmp/algorithms/SurfaceRemeshing.h>
#include <pmp/SurfaceMesh.h>
#include <unistd.h>

using namespace pmp;

void usage_and_exit()
{
    std::cerr << "\nExample usage\n-------------\n"
              << "hrtf_mesh_grading -x 0.5 -y 10 -s 'left' -i head.ply -o head_left.ply -v\n\n"
              << "Parameters\n----------\n"
              << "-x the minimum edge length in mm\n"
              << "-y the maximum edge length in mm\n"
              << "-e the maximum geometrical error in mm (Optional. The minimum edge length by default)\n"
              << "-s the side at which the mesh resolution will be high ('left' or 'right')\n"
              << "-g, h the scaling factor to estimate the y-coordinate of the left (g) and right (h) ear channel entrance (gamma on p. 1112 in Palm et al.). The default is 0.15. "
              << "Use this if the graded mesh contains to large or too small elements in the vicinity of the ear channels. Use the verbose flag to echo the gamma parameters."
              << "The estimated positions should have slightly smaller absolute values than the actual ear channel entrances.\n"
              << "-i the path to the input mesh\n"
              << "-o the path to the output mesh\n"
              << "-v verbose mode to echo input parameters and report mesh statistics (optional)\n"
              << "-b write the output mesh as binary data (optional)\n\n"
              << "Note\n----\n"
              << "Note the section 'Mesh Preparation' on https://github.com/cg-tub/hrtf_mesh_grading.\n\n"
              << "Reference\n---------\n"
              << "T. Palm, S. Koch, F. Brinkmann, and M. Alexa, “Curvature-adaptive mesh grading for numerical approximation of head-related transfer functions,” in DAGA 2021, Vienna, Austria, pp. 1111-1114.\n\n";

    exit(1);
}

int main(int argc, char** argv)
{
    bool binary = false;
    bool verbose = false;
    const char* input = nullptr;
    const char* output = nullptr;
    float min, max, err = 0;
    float gamma_scaling_left = 2.;
    float gamma_scaling_right = 2.;
    const char* ear = nullptr;

    // parse command line parameters ------------------------------------------
    int c;
    while ((c = getopt(argc, argv, "x:y:e:s:g:h:i:o:vi:bi:")) != -1)
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

            case 'g':
                gamma_scaling_left = std::stof(optarg);
                break;

            case 'h':
                gamma_scaling_right = std::stof(optarg);
                break;

            case 'i':
                input = optarg;
                break;

            case 'o':
                output = optarg;
                break;

            case 'v':
                verbose = true;
                break;

            case 'b':
                binary = true;
                break;

            default:
                usage_and_exit();
        }
    }

    // check input ------------------------------------------------------------
    if (min < 1e-6 || max < 1e-6 || !ear || !input || !output)
    {
        usage_and_exit();
    }

    // default parameters -----------------------------------------------------
    if (err < 1e-6)
    {
        err = min;
    }
    if (gamma_scaling_left > 1.9)
    {
        gamma_scaling_left = 0.15;
    }
    if (gamma_scaling_right > 1.9)
    {
        gamma_scaling_right = 0.15;
    }

    // echo input -------------------------------------------------------------
    if (verbose)
    {   std::cout << "\ninput: " << input << std::endl;
        std::cout << "output: " << output << std::endl;
        std::cout << "side: " << ear << std::endl;
        std::cout << "min. edge length: " << min << std::endl;
        std::cout << "max. edge length: " << max << std::endl;
        std::cout << "max. error: " << err << std::endl;
        std::cout << "gamma scaling left/right: "
            << gamma_scaling_left << "/" << gamma_scaling_right << std::endl;
    }

    // load input mesh --------------------------------------------------------
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

    const int faces_before = mesh.n_faces();

    // remeshing --------------------------------------------------------------
    SurfaceRemeshing(mesh).adaptive_remeshing(
                    min,  // min length
                    max,    // max length
                    err,  // approx. error
                    10U,
                    true,
                    ear,
                    gamma_scaling_left, gamma_scaling_right,
                    verbose
                    );

    // echo remeshing stats ---------------------------------------------------
    if (verbose)
    {
        std::cout << "\nFaces before remeshing: " << faces_before << std::endl;
        std::cout << "Faces after remeshing:  " << mesh.n_faces() << std::endl;
    }

    // write output mesh ------------------------------------------------------
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