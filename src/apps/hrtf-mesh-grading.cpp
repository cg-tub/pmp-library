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
              << "-l, r the left and right y-coordinate of the actual ear channel entrances in the unit of the input mesh. "
              << "Note that the gamma scaling factors won't be used if the actual positions are given.\n"
              << "-g, h the gamma scaling factor [1, p. 1112] to estimate the y-coordinate of the left (g) and right (h) ear channel entrance. The default is 0.15. "
              << "Use this if the actual ear channel entrance position is not know and the graded mesh contains to large or too small elements in the vicinity of the ear channels. "
              << "Use the verbose flag to echo the gamma parameters.\n"
              << "-d the value in mm to which the distance computation in [2, Eq. (2)] is normalized. If this is not passed it is computed from the mesh as is in [2], which might not be the best option for head and torso meshes.\n"
              << "-m mesh grading mode. 'hybrid' applies grading according to [1] (this ignores the -d parameter), 'distance' applies grading according to [2] (this ignores the -e parameter). The default is 'hybrid'\n"
              << "-i the path to the input mesh\n"
              << "-o the path to the output mesh\n"
              << "-v verbose mode to echo input parameters and report mesh statistics (optional)\n"
              << "-b write the output mesh as binary data (optional)\n\n"
              << "Note\n----\n"
              << "- Mind the section 'Mesh Preparation' and 'Trouble Shooting' on https://github.com/cg-tub/hrtf_mesh_grading.\n"
              << "- The parameters x, y, e, l, r, and d must be passed in mm. The unit of the mesh can be mm or m, which is automatically detected.\n"
              << "- Use the verbose flag to print the assumed ear channel positions. For the hybrid grading [1] the estimated positions should be slightly more inwards than the actual position (smaller absolute y-position). For the distance based grading [2] it should be exact.\n"
              << "- Passing anything other than 'left' or 'right' for the -s parameter will apply purely curvature based remeshing if the mode is 'hybrid' and use the minimum distance to the left and right ear channel for grading if the mode is 'distance'.\n\n"
              << "Reference\n---------\n"
              << "[1] T. Palm, S. Koch, F. Brinkmann, and M. Alexa, “Curvature-adaptive mesh grading for numerical approximation of head-related transfer functions,” in DAGA 2021, Vienna, Austria, pp. 1111-1114.\n"
              << "[2] H. Ziegelwanger, W. Kreuzer, and P. Majdak, “A-priori mesh grading for the numerical calculation of the head-related transfer functions,” Applied Acoustics 114:99-110, 2021. doi: 10.1016/j.apacoust.2016.07.005\n\n";;

    exit(1);
}

int main(int argc, char** argv)
{
    bool binary = false;
    bool verbose = false;
    const char* input = nullptr;
    const char* output = nullptr;
    float min, max, err = 0;
    float channel_left = 0.;
    float channel_right = 0.;
    float gamma_scaling_left = 2.;
    float gamma_scaling_right = 2.;
    float d_max = 0.;
    const char* ear = nullptr;
    std::string mode = "hybrid";

    // parse command line parameters ------------------------------------------
    int c;
    while ((c = getopt(argc, argv, "x:y:e:s:l:r:g:h:d:m:i:o:vi:bi:")) != -1)
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

            case 'l':
                channel_left = std::stof(optarg);
                break;

            case 'r':
                channel_right = std::stof(optarg);
                break;

            case 'g':
                gamma_scaling_left = std::stof(optarg);
                break;

            case 'h':
                gamma_scaling_right = std::stof(optarg);
                break;

            case 'd':
                d_max = std::stof(optarg);
                break;

            case 'm':
                mode = optarg;
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
        std::cout << "mode: " << mode << std::endl;
        std::cout << "side: " << ear << std::endl;
        std::cout << "min. edge length: " << min << std::endl;
        std::cout << "max. edge length: " << max << std::endl;
        std::cout << "max. error: " << err << std::endl;
        if (channel_left == 0. && channel_right == 0.)
        {
            std::cout << "gamma scaling left/right: "
                << gamma_scaling_left << "/" << gamma_scaling_right << std::endl;
        }

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
                    mode,
                    d_max,
                    channel_left,
                    channel_right,
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