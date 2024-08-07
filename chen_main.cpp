/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Benjamin Z. Chen

- All project requirements fully met? (YES or NO):
    YES

- If no, please explain what you could not get to work:
    Met all requirements, debugged why i couldnt get process_10 to work, but it was because of my main function logic, then did the same for process_11

- Did you do any optional enhancements? If so, please explain:
    Yes & no, I created a feature to turn the image into a pink vaporwave tint
*/

#include <iostream>  // for standard I/O operations
#include <vector>    // for using the vector container
#include <fstream>   // for file operations
#include <cmath>     // for mathematical functions
#include <algorithm> // for std::max and std::min and std::transform
#include <string>    // for std::string
#include <unistd.h>  // for getcwd
#include <limits.h>  // for PATH_MAX
#include <sstream>   // for std::stringstream
using namespace std; // for "std::" prefix

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

// function prototypes -- these are functions we will be using later
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
    cout << "_____________________" << endl;
    cout << "Image Processing Menu" << endl;
    cout << "1. Add Vignette" << endl;
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
    cout << "_____________________" << endl;
    cout << "Enter your choice: ";
}

// process 1: adding vignette (the measurements are from piazza, just copy them + follow python logic)
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

// process 2: clarendon effect (the scaling factor is what makes the intensity)
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

            int red_value = p.red;
            int green_value = p.green;
            int blue_value = p.blue;

            double average_value = (red_value + green_value + blue_value) / 3.0;

            int new_red, new_green, new_blue;

            if (average_value >= 170)
            {
                new_red = static_cast<int>(255 - (255 - red_value) * scaling_factor);
                new_green = static_cast<int>(255 - (255 - green_value) * scaling_factor);
                new_blue = static_cast<int>(255 - (255 - blue_value) * scaling_factor);
            }
            else if (average_value < 90)
            {
                new_red = static_cast<int>(red_value * scaling_factor);
                new_green = static_cast<int>(green_value * scaling_factor);
                new_blue = static_cast<int>(blue_value * scaling_factor);
            }
            else
            {
                new_red = red_value;
                new_green = green_value;
                new_blue = blue_value;
            }

            new_image[row][col] = {new_red, new_green, new_blue};
        }
    }

    return new_image;
}

// process 3: grayscale (copy piazza measurements + same logic, this 1 is straight forward)
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
            int gray_value = (p.red + p.green + p.blue) / 3;
            new_image[row][col] = {gray_value, gray_value, gray_value};
        }
    }

    return new_image;
}

// process 4: rotate 90 degrees clockwise - make sure it's NOT COUNTERCLOCKWISE
vector<vector<Pixel>> process_4(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            new_image[col][(height - 1) - row] = image[row][col];
        }
    }

    return new_image;
}

// helper function for rotating by 90 degrees, we will call this function in process 5 so that our code is cleaner and packaged well
vector<vector<Pixel>> rotate_by_90(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            new_image[col][(height - 1) - row] = image[row][col];
        }
    }

    return new_image;
}

// process 5: rotate multiples of 90 degrees clockwise NOT COUNTERCLOCKWISE - we will actually ask the user to input how much they wanna rotate
vector<vector<Pixel>> process_5(const vector<vector<Pixel>> &image)
{
    int num_rotations;
    cout << "enter the number of 90-degree clockwise rotations: ";
    cin >> num_rotations;

    num_rotations = num_rotations % 4;

    vector<vector<Pixel>> result_image = image;
    for (int i = 0; i < num_rotations; i++)
    {
        result_image = rotate_by_90(result_image);
    }
    return result_image;
}

// process 6: enlarge the image in the x and y direction
vector<vector<Pixel>> process_6(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    double xscale, yscale;

    cout << "enter the scaling factor for x (horizontal): ";
    cin >> xscale;
    cout << "enter the scaling factor for y (vertical): ";
    cin >> yscale;

    int new_width = static_cast<int>(width * xscale);
    int new_height = static_cast<int>(height * yscale);
    vector<vector<Pixel>> new_image(new_height, vector<Pixel>(new_width));

    for (int row = 0; row < new_height; ++row)
    {
        for (int col = 0; col < new_width; ++col)
        {
            new_image[row][col] = image[static_cast<int>(row / yscale)][static_cast<int>(col / xscale)];
        }
    }
    return new_image;
}

// process 7: convert to high contrast
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
            int gray_value = (p.red + p.green + p.blue) / 3;

            // where the high contrast magic is happening
            Pixel new_pixel;
            if (gray_value >= 255 / 2)
            {
                new_pixel = {255, 255, 255};
            }
            else
            {
                new_pixel = {0, 0, 0};
            }

            new_image[row][col] = new_pixel;
        }
    }

    return new_image;
}

// process 8: lighten the image by a scaling factor
vector<vector<Pixel>> process_8(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

    double scaling_factor = 0.5; // change this to any desired value -- piazza post on scaling is wrong, have to guess and check to match the example picture

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Pixel p = image[row][col];
            int new_red = static_cast<int>(255 - (255 - p.red) * scaling_factor);
            int new_green = static_cast<int>(255 - (255 - p.green) * scaling_factor);
            int new_blue = static_cast<int>(255 - (255 - p.blue) * scaling_factor);
            new_image[row][col] = {new_red, new_green, new_blue};
        }
    }

    return new_image;
}

// process 9: darken the image by a scaling factor
vector<vector<Pixel>> process_9(const vector<vector<Pixel>> &image)
{
    int width = image[0].size();
    int height = image.size();
    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

    double scaling_factor = 0.5; // change this to any desired value, i changed to 0.5 because it was closest to the example picture (the piazza measurement is wrong)

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Pixel p = image[row][col];
            int new_red = static_cast<int>(p.red * scaling_factor);
            int new_green = static_cast<int>(p.green * scaling_factor);
            int new_blue = static_cast<int>(p.blue * scaling_factor);
            new_image[row][col] = {new_red, new_green, new_blue};
        }
    }

    return new_image;
}

// process 10: convert to black, white, red, blue, and green - the picture is really intense, hardly see green in the example
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
            Pixel new_pixel;

            int red_value = current_pixel.red;
            int green_value = current_pixel.green;
            int blue_value = current_pixel.blue;
            int max_color = max({red_value, green_value, blue_value});

            if (red_value + green_value + blue_value >= 550)
            {
                // full white
                new_pixel.red = 255;
                new_pixel.green = 255;
                new_pixel.blue = 255;
            }
            else if (red_value + green_value + blue_value <= 150)
            {
                // full dark
                new_pixel.red = 0;
                new_pixel.green = 0;
                new_pixel.blue = 0;
            }
            else if (max_color == red_value)
            {
                // red
                new_pixel.red = 255;
                new_pixel.green = 0;
                new_pixel.blue = 0;
            }
            else if (max_color == green_value)
            {
                // green
                new_pixel.red = 0;
                new_pixel.green = 255;
                new_pixel.blue = 0;
            }
            else
            {
                // blue
                new_pixel.red = 0;
                new_pixel.green = 0;
                new_pixel.blue = 255;
            }

            processed_image[i][j] = new_pixel;
        }
    }

    return processed_image;
}

// process 11: turn image into sailormoon vaporwave pink
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

            // apply a cream pink tint, have to guess and check the severity
            new_pixel.red = min(255, static_cast<int>(current_pixel.red * 1.1 + 100));
            new_pixel.green = min(255, static_cast<int>(current_pixel.green * 0.8 + 70));
            new_pixel.blue = min(255, static_cast<int>(current_pixel.blue * 0.8 + 70));

            processed_image[i][j] = new_pixel;
        }
    }

    return processed_image;
}

bool ends_with(const std::string &str, const std::string &suffix)
{
    if (str.size() < suffix.size())
        return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

int main()
{
    char quit_choice;
    int choice;
    string input_file, output_file;
    vector<vector<Pixel>> image;

    while (true)
    {
        cout << "enter the your BMP filename (must end with .bmp): ";
        cin >> input_file;
        if (ends_with(input_file, ".bmp"))
        {
            break;
        }
        else
        {
            cerr << "error, filename must end with .bmp" << endl;
        }
    }

    while (true)
    {
        display_menu();
        cin >> quit_choice;

        if (quit_choice == 'Q' || quit_choice == 'q')
        {
            cout << "thank you, come again... quitting!" << endl;
            break;
        }

        if (quit_choice == 'C' || quit_choice == 'c')
        {
            while (true)
            {
                cout << "enter your new input BMP filename (must end with .bmp): ";
                cin >> input_file;
                if (ends_with(input_file, ".bmp"))
                {
                    break;
                }
                else
                {
                    cerr << "error, filename must end with .bmp" << endl;
                }
            }
            continue;
        }

        if (isdigit(quit_choice))
        {
            choice = quit_choice - '0';

            if (choice == 1)
            {
                char next_digit;
                cin.get(next_digit);
                if (next_digit == '0')
                {
                    choice = 10;
                }
                else if (next_digit == '1')
                {
                    choice = 11;
                }
                else
                {
                    cin.putback(next_digit);
                }
            }
        }
        else
        {
            cerr << "invalid choice, try again." << endl;
            continue;
        }

        if (choice < 1 || choice > 11)
        {
            cerr << "invalid choice, try again." << endl;
            continue;
        }

        while (true)
        {
            cout << "enter the output BMP filename (must end with .bmp): ";
            cin >> output_file;
            if (ends_with(output_file, ".bmp"))
            {
                break;
            }
            else
            {
                cerr << "error, the filename must end with .bmp" << endl;
            }
        }

        image = read_image(input_file);
        if (image.empty())
        {
            cerr << "error, could not read file " << input_file << endl;
            continue;
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
            cerr << "invalid choice, try again." << endl;
            continue;
        }

        if (!write_image(output_file, processed_image))
        {
            cerr << "error, could not write file " << output_file << endl;
        }
        else
        {
            cout << "altered image saved as " << output_file << " " << endl;
        }
    }

    return 0;
}