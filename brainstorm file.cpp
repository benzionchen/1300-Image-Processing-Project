/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Benjamin Z. Chen

- All project requirements fully met? (YES or NO):
    YES

- If no, please explain what you could not get to work:
    Met all requirements

- Did you do any optional enhancements? If so, please explain:
    Yes, created a feature to prompt the user which instagram filter to use on the the 10 images (like turn it into a rose filter, or vignette, or invert colors, etc.)
*/

#include <iostream>  // Includes the input-output stream library for standard I/O operations
#include <vector>    // Includes the vector library to use the vector container
#include <fstream>   // Includes the file stream library for file operations
#include <cmath>     // Includes the cmath library for mathematical functions
#include <string>    // For std::string
#include <unistd.h>  // For getcwd
#include <limits.h>  // For PATH_MAX
using namespace std; // Use standard library names without the "std::" prefix

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// pixel structure
struct Pixel
{
    // Red, Green, Blue color values
    int red;
    int green;
    int blue;
};

/**
 * gets an integer from a binary stream.
 * helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */
int get_int(fstream &stream, int offset, int bytes)
{
    stream.seekg(offset); // moves pointer to the specified offset
    int result = 0;       // initializes the result to 0, start counter
    int base = 1;         // initializes the base multiplier to 1, start counter
    for (int i = 0; i < bytes; i++)
    {
        result = result + stream.get() * base; // reads a byte from the stream, multiplies it by the base, and adds it to the result
        base = base * 256;                     // increases the base by a factor of 256 (2^8) for the next byte
    }
    return result; // Returns the final integer value
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary); // opens the file in read mode

    // get the image properties and dimensions
    int file_size = get_int(stream, 2, 4);       // reads the file size from offset 2 (4 bytes)
    int start = get_int(stream, 10, 4);          // start offset of the pixel array from offset 10 (4 bytes)
    int width = get_int(stream, 18, 4);          // image width from offset 18 (4 bytes)
    int height = get_int(stream, 22, 4);         // image height from offset 22 (4 bytes)
    int bits_per_pixel = get_int(stream, 28, 2); // bits per pixel from offset 28 (2 bytes)

    // scan lines must occupy multiples of 4-bytes
    int scanline_size = width * (bits_per_pixel / 8); // Calculates the size of each scanline (row of pixels)
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4; // Calculates the padding needed to make the scanline size a multiple of 4
    }

    // return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {}; // returns an empty vector if the file size does not match the expected size (this should not throw error, should just be empty...?) - remember to check
    }

    // create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel>(width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // iterating on each column
        for (int j = 0; j < width; j++)
        {
            // go to the pixel position
            stream.seekg(pos);

            // save the pixel values to the image vector
            // note: BMP files store pixels in blue, green, red order (need to remember this for later)
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // we are ignoring the alpha channel if there is one

            // advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // skip the padding at each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // close the stream + return vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset + i] = (unsigned char)(value >> (i * 8)); // Sets each byte of the array to the corresponding byte of the value
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>> &image)
{
    // get the image width and height (make sure it is in pixels)
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // calculate the width in bytes incorporating padding (4-byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // open filestream to access binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // return false if the file has problems opening
    if (!stream.is_open())
    {
        return false;
    }

    // create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header properties
    set_bytes(bmp_header, 0, 1, 'B');                                             // ID field
    set_bytes(bmp_header, 1, 1, 'M');                                             // ID field
    set_bytes(bmp_header, 2, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE + array_bytes); // size of BMP file
    set_bytes(bmp_header, 6, 2, 0);                                               // reserved
    set_bytes(bmp_header, 8, 2, 0);                                               // reserved again
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE);              // pixel array offset

    // DIB Header properties
    set_bytes(dib_header, 0, 4, DIB_HEADER_SIZE); // DIB header size
    set_bytes(dib_header, 4, 4, width_pixels);    // width of bitmap in pixels
    set_bytes(dib_header, 8, 4, height_pixels);   // height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);              // number of color planes
    set_bytes(dib_header, 14, 2, 24);             // number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);              // compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);    // size of raw bitmap data (including padding)
    set_bytes(dib_header, 24, 4, 2835);           // print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);           // print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);              // number of colors in palette
    set_bytes(dib_header, 36, 4, 0);              // number of important colors

    // write the BMP and DIB Headers to the file
    stream.write((char *)bmp_header, sizeof(bmp_header));
    stream.write((char *)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char *)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//

// function to process the image by inverting the colors of each pixel
vector<vector<Pixel>> invert_colors(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> processed_image(num_rows, vector<Pixel>(num_columns));
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_columns; j++)
        {
            Pixel current_pixel = image[i][j];
            Pixel new_pixel;
            new_pixel.red = 255 - current_pixel.red;
            new_pixel.green = 255 - current_pixel.green;
            new_pixel.blue = 255 - current_pixel.blue;
            processed_image[i][j] = new_pixel;
        }
    }
    return processed_image;
}

// function to process the image by tinting it into sailormoon pink
// to turn it pink, you need rgb values at 100% red, 75.3% green, 79.6% blue
vector<vector<Pixel>> tint_pink(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> processed_image(num_rows, vector<Pixel>(num_columns));
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_columns; j++)
        {
            Pixel current_pixel = image[i][j];
            Pixel new_pixel;
            new_pixel.red = min(255, current_pixel.red + 50);   // after many trial and error, this is the closest i can get to pink without it looking like brown/orange
            new_pixel.green = max(0, current_pixel.green - 30); // this was previously 100, -30, -50
            new_pixel.blue = max(0, current_pixel.blue - 50);
            processed_image[i][j] = new_pixel;
        }
    }
    return processed_image;
}

// function to apply a vignette effect to the image
vector<vector<Pixel>> apply_vignette(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> processed_image(num_rows, vector<Pixel>(num_columns));
    double center_x = num_columns / 2.0;
    double center_y = num_rows / 2.0;
    double max_distance = sqrt(center_x * center_x + center_y * center_y);

    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_columns; j++)
        {
            Pixel current_pixel = image[i][j];
            double distance = sqrt((j - center_x) * (j - center_x) + (i - center_y) * (i - center_y));
            double vignette_factor = 1.0 - (distance / max_distance);
            vignette_factor = pow(vignette_factor, 2.0);
            Pixel new_pixel;
            new_pixel.red = current_pixel.red * vignette_factor;
            new_pixel.green = current_pixel.green * vignette_factor;
            new_pixel.blue = current_pixel.blue * vignette_factor;
            processed_image[i][j] = new_pixel;
        }
    }
    return processed_image;
}

int main()
{
    // fetch the current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) // if there exists a cwd, we should grab it
    {
        cout << "current working directory: " << cwd << endl;
    }
    else
    {
        cerr << "error: problem with finding the cwd" << endl; // throw an error if there is for any reason a problem with returning the path name
        return 1;
    }

    // ask/prompt the user on what filter to apply to the images
    cout << "enter the number corresponding to the filter you want to use on the image:" << endl;
    cout << "1. invert the image colors" << endl;
    cout << "2. tint a vaporwave pink" << endl;
    cout << "3. apply a shadow-y vignette" << endl;
    int choice; // make sure that the user is not entering a string, and make sure that the choice is valid, as in not entering things like strings or integers above 3 or negatives
    cin >> choice;

    // iterate over each image file (this part is hardcoded, but it's fine because we know there are 10 files in the folder)
    for (int i = 1; i <= 10; ++i)
    {
        string input_filename = "sample_images/process" + to_string(i) + ".bmp";
        string output_filename = "sample_images/output" + to_string(i) + ".bmp";

        // read image file and turn into a 2D vector
        vector<vector<Pixel>> image = read_image(input_filename);

        // check if the image was read successfully, create an error in case it doesn't read properly
        if (image.empty())
        {
            cerr << "error: couldn't read the image file " << input_filename << endl; // honetsly this should never happen but good habit to write this test error
            continue;
        }

        // process the image based on the user's choice
        vector<vector<Pixel>> processed_image;
        switch (choice)
        {
        case 1:
            processed_image = invert_colors(image);
            break;
        case 2:
            processed_image = tint_pink(image);
            break;
        case 3:
            processed_image = apply_vignette(image);
            break;
        default:
            cerr << "invalid choice, the image will not be processed" << endl;
            continue;
        }

        // write the resulting 2D vector to a new BMP image file
        if (!write_image(output_filename, processed_image))
        {
            cerr << "error: could not create new image " << output_filename << endl;
            continue;
        }

        cout << "altered the image " << input_filename << " and saved to " << output_filename << endl;
    }

    cout << "voila! done!" << endl;
    return 0;
}

int main()
{
    // Get the current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        cout << "Current working directory: " << cwd << endl;
    }
    else
    {
        cerr << "Error: Could not get the current working directory" << endl;
        return 1;
    }

    // Prompt the user for the desired action
    cout << "Choose an action:" << endl;
    cout << "1. Alter a sample image" << endl;
    cout << "2. Apply a filter to all output images" << endl;
    int action_choice;
    cin >> action_choice;

    if (action_choice == 1)
    {
        // User chooses to alter a sample image
        cout << "Choose an image processing option:" << endl;
        cout << "1. Process 1 (Add Vignette)" << endl;
        cout << "2. Process 2 (Add Clarendon Effect)" << endl;
        cout << "3. Process 3 (Add Grayscale)" << endl;
        cout << "4. Process 4 (Rotate 90 Degrees Clockwise)" << endl;
        cout << "5. Process 5 (Rotate Multiple of 90 Degrees Clockwise)" << endl;
        cout << "6. Process 6 (Enlarge Image)" << endl;
        cout << "7. Process 7 (High Contrast)" << endl;
        cout << "8. Process 8 (Lighten Image)" << endl;
        cout << "9. Process 9 (Darken Image)" << endl;
        cout << "10. Process 10 (Convert to BW/R/G/B)" << endl;
        int process_choice;
        cin >> process_choice;

        string input_filename = "sample_images/sample.bmp";
        string output_filename = "sample_images/output_sample.bmp";

        // Read in BMP image file into a 2D vector
        vector<vector<Pixel>> image = read_image(input_filename);

        // Check if the image was read successfully
        if (image.empty())
        {
            cerr << "Error: Could not read the image file " << input_filename << endl;
            return 1;
        }

        // Process the image based on the user's choice
        vector<vector<Pixel>> processed_image;
        switch (process_choice)
        {
        case 1:
            processed_image = process_1(image);
            break;
        case 2:
            processed_image = process_2(image);
            break;
        case 3:
            processed_image = process_3(image);
            break;
        case 4:
            processed_image = process_4(image);
            break;
        case 5:
            processed_image = process_5(image);
            break;
        case 6:
            processed_image = process_6(image);
            break;
        case 7:
            processed_image = process_7(image);
            break;
        case 8:
            processed_image = process_8(image);
            break;
        case 9:
            processed_image = process_9(image);
            break;
        case 10:
            processed_image = process_10(image);
            break;
        default:
            cerr << "Invalid choice. Exiting." << endl;
            return 1;
        }

        // Write the resulting 2D vector to a new BMP image file
        if (!write_image(output_filename, processed_image))
        {
            cerr << "Error: Could not write to the image file " << output_filename << endl;
            return 1;
        }

        cout << "Processed image " << input_filename << " and saved to " << output_filename << endl;
    }
    else if (action_choice == 2)
    {
        // User chooses to apply a filter to all output images
        cout << "Choose an image processing filter:" << endl;
        cout << "1. Invert Colors" << endl;
        cout << "2. Tint Pink" << endl;
        cout << "3. Apply Vignette" << endl;
        int filter_choice;
        cin >> filter_choice;

        for (int i = 1; i <= 10; ++i)
        {
            string input_filename = "sample_images/process" + to_string(i) + ".bmp";
            string output_filename = "sample_images/output" + to_string(i) + ".bmp";

            // Read in BMP image file into a 2D vector
            vector<vector<Pixel>> image = read_image(input_filename);

            // Check if the image was read successfully
            if (image.empty())
            {
                cerr << "Error: Could not read the image file " << input_filename << endl;
                continue;
            }

            // Process the image based on the user's choice
            vector<vector<Pixel>> processed_image;
            switch (filter_choice)
            {
            case 1:
                processed_image = invert_colors(image);
                break;
            case 2:
                processed_image = tint_pink(image);
                break;
            case 3:
                processed_image = apply_vignette(image);
                break;
            default:
                cerr << "Invalid choice. Skipping processing." << endl;
                continue;
            }

            // Write the resulting 2D vector to a new BMP image file
            if (!write_image(output_filename, processed_image))
            {
                cerr << "Error: Could not write to the image file " << output_filename << endl;
                continue;
            }

            cout << "Processed image " << input_filename << " and saved to " << output_filename << endl;
        }
    }
    else
    {
        cerr << "Invalid action choice. Exiting." << endl;
        return 1;
    }

    cout << "Image processing complete." << endl;
    return 0;
}

// Function to process the image by inverting the colors of each pixel
vector<vector<Pixel>> invert_colors(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> processed_image(num_rows, vector<Pixel>(num_columns));
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_columns; j++)
        {
            Pixel current_pixel = image[i][j];
            Pixel new_pixel;
            new_pixel.red = 255 - current_pixel.red;
            new_pixel.green = 255 - current_pixel.green;
            new_pixel.blue = 255 - current_pixel.blue;
            processed_image[i][j] = new_pixel;
        }
    }
    return processed_image;
}

// Function to process the image by tinting it pink
vector<vector<Pixel>> tint_pink(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> processed_image(num_rows, vector<Pixel>(num_columns));
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_columns; j++)
        {
            Pixel current_pixel = image[i][j];
            Pixel new_pixel;
            new_pixel.red = min(255, current_pixel.red + 70);     // Slight increase in red for a very pale pink
            new_pixel.green = min(255, current_pixel.green + 50); // Small increase in green for a pastel effect
            new_pixel.blue = min(255, current_pixel.blue + 60);   // Small increase in blue for balance
            processed_image[i][j] = new_pixel;
        }
    }
    return processed_image;
}

// Function to apply a vignette effect to the image
vector<vector<Pixel>> apply_vignette(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> processed_image(num_rows, vector<Pixel>(num_columns));
    double center_x = num_columns / 2.0;
    double center_y = num_rows / 2.0;
    double max_distance = sqrt(center_x * center_x + center_y * center_y);

    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_columns; j++)
        {
            Pixel current_pixel = image[i][j];
            double distance = sqrt((j - center_x) * (j - center_x) + (i - center_y) * (i - center_y));
            double vignette_factor = 1.0 - (distance / max_distance);
            vignette_factor = pow(vignette_factor, 2.0);
            Pixel new_pixel;
            new_pixel.red = current_pixel.red * vignette_factor;
            new_pixel.green = current_pixel.green * vignette_factor;
            new_pixel.blue = current_pixel.blue * vignette_factor;
            processed_image[i][j] = new_pixel;
        }
    }
    return processed_image;
}
