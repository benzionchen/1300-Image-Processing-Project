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

// Function prototypes
vector<vector<Pixel>> read_image(string filename);
bool write_image(string filename, const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_1(const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_2(const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_3(const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_4(const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_5(const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_6(const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_7(const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_8(const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_9(const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_10(const vector<vector<Pixel>> &image);
vector<vector<Pixel>> process_11(const vector<vector<Pixel>> &image);

vector<vector<Pixel>> rotate_90_degrees(const vector<vector<Pixel>> &image);

void display_menu()
{
    cout << "Image Processing Menu" << endl;
    cout << "1. Add vignette" << endl;
    cout << "2. Add Clarendon effect" << endl;
    cout << "3. Convert to grayscale" << endl;
    cout << "4. Rotate image 90 degrees clockwise" << endl;
    cout << "5. Rotate image multiples of 90 degrees clockwise" << endl;
    cout << "6. Enlarge the image in the x and y direction" << endl;
    cout << "7. Convert image to high contrast" << endl;
    cout << "8. Lighten the image by a scaling factor" << endl;
    cout << "9. Darken the image by a scaling factor" << endl;
    cout << "10. Convert image to black, white, red, blue, and green" << endl;
    cout << "11. Turn image into cream pink" << endl;
    cout << "Q. Quit" << endl;
    cout << "Enter your choice: ";
}

vector<vector<Pixel>> process_1(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    double center_x = num_columns / 2.0;
    double center_y = num_rows / 2.0;

    for (int row = 0; row < num_rows; ++row)
    {
        for (int col = 0; col < num_columns; ++col)
        {
            Pixel p = image[row][col];
            double distance = sqrt(pow(col - center_x, 2) + pow(row - center_y, 2));
            double scaling_factor = (num_rows - distance) / num_rows;
            int new_red = static_cast<int>(p.red * scaling_factor);
            int new_green = static_cast<int>(p.green * scaling_factor);
            int new_blue = static_cast<int>(p.blue * scaling_factor);
            new_image[row][col] = {new_red, new_green, new_blue};
        }
    }
    return new_image;
}

vector<vector<Pixel>> process_2(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    double scaling_factor = 0.3;
    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Pixel p = image[row][col];

            // Get the red, green, and blue values
            int red_value = p.red;
            int green_value = p.green;
            int blue_value = p.blue;

            // Calculate the average value
            double average_value = (red_value + green_value + blue_value) / 3.0;

            // Apply Clarendon effect
            int new_red, new_green, new_blue;

            // Light cells
            if (average_value >= 170)
            {
                new_red = static_cast<int>(255 - (255 - red_value) * scaling_factor);
                new_green = static_cast<int>(255 - (255 - green_value) * scaling_factor);
                new_blue = static_cast<int>(255 - (255 - blue_value) * scaling_factor);
            }
            // Dark cells
            else if (average_value < 90)
            {
                new_red = static_cast<int>(red_value * scaling_factor);
                new_green = static_cast<int>(green_value * scaling_factor);
                new_blue = static_cast<int>(blue_value * scaling_factor);
            }
            // Mid-range cells
            else
            {
                new_red = red_value;
                new_green = green_value;
                new_blue = blue_value;
            }

            // Set the new pixel in the new image
            new_image[row][col] = {new_red, new_green, new_blue};
        }
    }

    return new_image;
}

vector<vector<Pixel>> process_3(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Pixel p = image[row][col];

            // Get the red, green, and blue values
            int red_value = p.red;
            int green_value = p.green;
            int blue_value = p.blue;

            // Calculate the grayscale value
            int gray_value = (red_value + green_value + blue_value) / 3;

            // Set new color values to be the grayscale value
            Pixel new_pixel;
            new_pixel.red = gray_value;
            new_pixel.green = gray_value;
            new_pixel.blue = gray_value;

            // Set the new pixel in the new image
            new_image[row][col] = new_pixel;
        }
    }

    return new_image;
}

vector<vector<Pixel>> process_4(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height)); // Note the width and height are swapped

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Pixel p = image[row][col];
            new_image[col][(height - 1) - row] = p; // Rotate 90 degrees clockwise
        }
    }

    return new_image;
}

vector<vector<Pixel>> rotate_by_90(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height)); // Note the width and height are swapped

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Pixel p = image[row][col];
            new_image[col][(height - 1) - row] = p; // Rotate 90 degrees clockwise
        }
    }

    return new_image;
}

vector<vector<Pixel>> process_5(const vector<vector<Pixel>> &image)
{
    int num_rotations;
    cout << "Enter the number of 90-degree rotations (clockwise): ";
    cin >> num_rotations;

    // Normalize the number of rotations to be between 0 and 3
    num_rotations = num_rotations % 4;

    if (num_rotations == 0)
    {
        return image;
    }
    else if (num_rotations == 1)
    {
        return rotate_by_90(image);
    }
    else if (num_rotations == 2)
    {
        return rotate_by_90(rotate_by_90(image));
    }
    else
    {
        return rotate_by_90(rotate_by_90(rotate_by_90(image)));
    }
}

vector<vector<Pixel>> process_6(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    double xscale, yscale;

    cout << "Enter the scaling factor for x direction: ";
    cin >> xscale;
    cout << "Enter the scaling factor for y direction: ";
    cin >> yscale;

    int new_width = static_cast<int>(width * xscale);
    int new_height = static_cast<int>(height * yscale);
    vector<vector<Pixel>> new_image(new_height, vector<Pixel>(new_width));

    for (int row = 0; row < new_height; ++row)
    {
        for (int col = 0; col < new_width; ++col)
        {
            Pixel p = image[static_cast<int>(row / yscale)][static_cast<int>(col / xscale)];
            new_image[row][col] = p;
        }
    }

    return new_image;
}

vector<vector<Pixel>> process_7(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Pixel p = image[row][col];
            int red_value = p.red;
            int green_value = p.green;
            int blue_value = p.blue;

            // Average the RGB values to get the grayscale value
            int gray_value = (red_value + green_value + blue_value) / 3;

            Pixel new_pixel;
            if (gray_value >= 255 / 2)
            {
                new_pixel.red = 255;
                new_pixel.green = 255;
                new_pixel.blue = 255;
            }
            else
            {
                new_pixel.red = 0;
                new_pixel.green = 0;
                new_pixel.blue = 0;
            }

            new_image[row][col] = new_pixel;
        }
    }

    return new_image;
}

vector<vector<Pixel>> process_8(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

    double scaling_factor = 0.5; // You can change this to any desired value

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Pixel p = image[row][col];
            int red_value = p.red;
            int green_value = p.green;
            int blue_value = p.blue;

            int new_red = static_cast<int>(255 - (255 - red_value) * scaling_factor);
            int new_green = static_cast<int>(255 - (255 - green_value) * scaling_factor);
            int new_blue = static_cast<int>(255 - (255 - blue_value) * scaling_factor);

            Pixel new_pixel;
            new_pixel.red = new_red;
            new_pixel.green = new_green;
            new_pixel.blue = new_blue;

            new_image[row][col] = new_pixel;
        }
    }

    return new_image;
}

vector<vector<Pixel>> process_9(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

    double scaling_factor = 0.5; // You can change this to any desired value

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Pixel p = image[row][col];
            int red_value = p.red;
            int green_value = p.green;
            int blue_value = p.blue;

            int new_red = static_cast<int>(red_value * scaling_factor);
            int new_green = static_cast<int>(green_value * scaling_factor);
            int new_blue = static_cast<int>(blue_value * scaling_factor);

            Pixel new_pixel;
            new_pixel.red = new_red;
            new_pixel.green = new_green;
            new_pixel.blue = new_blue;

            new_image[row][col] = new_pixel;
        }
    }

    return new_image;
}

vector<vector<Pixel>> process_10(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> processed_image(num_rows, vector<Pixel>(num_columns));

    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_columns; j++)
        {
            Pixel current_pixel = image[i][j];
            int red = current_pixel.red;
            int green = current_pixel.green;
            int blue = current_pixel.blue;

            int max_color = max({red, green, blue});
            Pixel new_pixel;

            if (red + green + blue >= 550)
            {
                new_pixel.red = 255;
                new_pixel.green = 255;
                new_pixel.blue = 255;
            }
            else if (red + green + blue <= 150)
            {
                new_pixel.red = 0;
                new_pixel.green = 0;
                new_pixel.blue = 0;
            }
            else if (max_color == red)
            {
                new_pixel.red = 255;
                new_pixel.green = 0;
                new_pixel.blue = 0;
            }
            else if (max_color == green)
            {
                new_pixel.red = 0;
                new_pixel.green = 255;
                new_pixel.blue = 0;
            }
            else
            {
                new_pixel.red = 0;
                new_pixel.green = 0;
                new_pixel.blue = 255;
            }

            processed_image[i][j] = new_pixel;
        }
    }

    return processed_image;
}

vector<vector<Pixel>> process_11(const vector<vector<Pixel>> &image)
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
            new_pixel.red = min(255, static_cast<int>(current_pixel.red * 1.2 + 200 * 0.2));     // Slight increase in red for a very pale pink
            new_pixel.green = min(255, static_cast<int>(current_pixel.green * 0.9 + 200 * 0.1)); // Small decrease in green for a pastel effect
            new_pixel.blue = min(255, static_cast<int>(current_pixel.blue * 0.9 + 255 * 0.1));   // Small decrease in blue for balance
            processed_image[i][j] = new_pixel;
        }
    }
    return processed_image;
}

int main()
{
    char quit_choice;
    int choice;
    string input_file = "sample_images/sample.bmp";
    string output_file = "sample_images/output.bmp";
    vector<vector<Pixel>> image;

    while (true)
    {
        display_menu();
        cin >> quit_choice;

        if (quit_choice == 'Q' || quit_choice == 'q')
        {
            cout << "Thank you for using my program, quitting!" << endl;
            break;
        }
        else
        {
            choice = quit_choice - '0'; // Convert char to int
            if (choice < 1 || choice > 11)
            {
                cerr << "Invalid choice, please try again." << endl;
                continue;
            }
        }

        if (choice != 11)
        {
            image = read_image(input_file);
            if (image.empty())
            {
                cerr << "Error: Could not read the image file " << input_file << endl;
                continue;
            }
        }

        vector<vector<Pixel>> processed_image;
        switch (choice)
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
        case 11:
            processed_image = process_11(image);
            break;
        default:
            cerr << "Invalid choice, please try again." << endl;
            continue;
        }

        if (!write_image(output_file, processed_image))
        {
            cerr << "Error: Could not write the image file " << output_file << endl;
        }
        else
        {
            cout << "Processed image saved to " << output_file << " " << endl;
        }
    }

    return 0;
}