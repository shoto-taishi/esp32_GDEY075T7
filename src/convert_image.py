from PIL import Image, ImageTk
import numpy as np
import tkinter as tk
from tkinter import filedialog, messagebox, Label, Scale, Button

def resize_and_pad(image, target_width, target_height):
    """
    Resizes the image while maintaining its aspect ratio and pads it to the target dimensions.
    """
    original_width, original_height = image.size

    # Calculate the new dimensions preserving the aspect ratio
    aspect_ratio = original_width / original_height
    if aspect_ratio > (target_width / target_height):
        new_width = target_width
        new_height = int(target_width / aspect_ratio)
    else:
        new_height = target_height
        new_width = int(target_height * aspect_ratio)

    # Resize the image
    resized_image = image.resize((new_width, new_height), Image.Resampling.LANCZOS)

    # Create a new black image with the target dimensions
    new_image = Image.new("L", (target_width, target_height), 0)

    # Calculate the position to paste the resized image to center it
    x_offset = (target_width - new_width) // 2
    y_offset = (target_height - new_height) // 2

    # Paste the resized image onto the black image
    new_image.paste(resized_image, (x_offset, y_offset))

    return new_image

def convert_to_grayscale_bitmaps(image, thresholds, target_width=800, target_height=480):
    """
    Converts the image to two bitmaps representing 4-level grayscale.
    """
    # Convert to grayscale
    image = image.convert('L')

    # Convert the grayscale image to a numpy array
    image_np = np.array(image)

    # Initialize two byte arrays for the bitmaps
    bitmap1 = bytearray()
    bitmap2 = bytearray()

    for y in range(target_height):
        for x in range(0, target_width, 8):
            byte1 = 0
            byte2 = 0
            for bit in range(8):
                if x + bit < target_width:
                    pixel_value = image_np[y, x + bit]
                    if pixel_value < thresholds[0]:
                        bit_value1 = 0
                        bit_value2 = 0  # black
                    elif pixel_value < thresholds[1]:
                        bit_value1 = 0
                        bit_value2 = 1  # dark grey
                    elif pixel_value < thresholds[2]:
                        bit_value1 = 1
                        bit_value2 = 0  # light grey
                    else:
                        bit_value1 = 1
                        bit_value2 = 1  # white

                    byte1 = byte1 | (bit_value1 << (7 - bit))
                    byte2 = byte2 | (bit_value2 << (7 - bit))
            bitmap1.append(byte1)
            bitmap2.append(byte2)

    return bitmap1, bitmap2

def save_bitmaps_to_file(bitmap1, bitmap2, file_path, variable_name1="bitmap1", variable_name2="bitmap2"):
    """
    Saves the bitmaps to a text file in the specified format.
    """
    with open(file_path, 'w') as file:
        file.write(f"const unsigned char {variable_name1}[{len(bitmap1)}] = {{\n")
        for i in range(0, len(bitmap1), 16):
            line_data = bitmap1[i:i + 16]
            hex_values = ','.join(f"0X{byte:02X}" for byte in line_data)
            file.write(f"  {hex_values},\n")
        file.write("};\n\n")

        file.write(f"const unsigned char {variable_name2}[{len(bitmap2)}] = {{\n")
        for i in range(0, len(bitmap2), 16):
            line_data = bitmap2[i:i + 16]
            hex_values = ','.join(f"0X{byte:02X}" for byte in line_data)
            file.write(f"  {hex_values},\n")
        file.write("};\n")

def process_image(input_image_path, thresholds, rotate_angle=0):
    # Load the image
    image = Image.open(input_image_path)

    # Rotate the image if needed
    if rotate_angle != 0:
        image = image.rotate(rotate_angle, expand=True)

    w = image.width
    h = image.height
    
    # image = image.resize((100,60), resample=Image.Resampling.BILINEAR)
    # image = image.resize((w,h), resample=Image.Resampling.NEAREST)

    # Resize and pad the image to the required dimensions
    processed_image = resize_and_pad(image, 800, 480)

    # Convert to grayscale bitmaps
    bitmap1, bitmap2 = convert_to_grayscale_bitmaps(processed_image, thresholds)

    return bitmap1, bitmap2

def update_preview(image, thresholds):
    # Apply thresholds and generate preview
    image = image.convert('L')
    image_np = np.array(image)
    levels = np.digitize(image_np, thresholds, right=True)

    # Create a preview image
    preview_image = np.zeros_like(image_np)
    preview_image[levels == 0] = 0   # Black
    preview_image[levels == 1] = 85  # Dark grey
    preview_image[levels == 2] = 170 # Light grey
    preview_image[levels == 3] = 255 # White

    preview_image = Image.fromarray(preview_image)
    return preview_image

def select_image():
    file_path = filedialog.askopenfilename(
        title="Select an image",
        filetypes=[("Image files", "*.jpg;*.jpeg;*.png;*.bmp;*.gif")]
    )
    if file_path:
        input_path_entry.delete(0, tk.END)
        input_path_entry.insert(0, file_path)
        load_image(file_path)

def load_image(file_path):
    global original_image, rotate_angle
    original_image = Image.open(file_path)
    rotate_angle = 0  # Reset rotation angle when a new image is loaded
    display_image(original_image)

def display_image(image):
    global photo
    image = image.resize((400, 240), Image.Resampling.LANCZOS)
    photo = ImageTk.PhotoImage(image)
    image_label.config(image=photo)
    image_label.image = photo

def update_preview_image(*args):
    if original_image:
        thresholds = [slider1.get(), slider2.get(), slider3.get()]
        rotated_image = original_image.rotate(rotate_angle, expand=True)
        resized_image = resize_and_pad(rotated_image, 800, 480)
        preview_image = update_preview(resized_image, thresholds)
        display_image(preview_image)

def rotate_image():
    global original_image, rotate_angle
    if original_image:
        rotate_angle = (rotate_angle + 90) % 360
        rotated_image = original_image.rotate(rotate_angle, expand=True)
        display_image(rotated_image)
        update_preview_image()

def convert_and_save():
    input_image_path = input_path_entry.get()
    if not input_image_path:
        messagebox.showerror("Error", "Please select an image file.")
        return

    thresholds = [slider1.get(), slider2.get(), slider3.get()]
    bitmap1, bitmap2 = process_image(input_image_path, thresholds, rotate_angle)
    output_file_path = "image.h"  # Specify your desired output file path
    save_bitmaps_to_file(bitmap1, bitmap2, output_file_path)
    messagebox.showinfo("Success", f"Image processed and saved to {output_file_path}.")

# Create the main window
root = tk.Tk()
root.title("Image to Grayscale Bitmap Converter")

original_image = None
rotate_angle = 0  # Store the current rotation angle

# Create and place the input path entry
input_path_entry = tk.Entry(root, width=50)
input_path_entry.grid(row=0, column=0, padx=10, pady=10)

# Create and place the browse button
browse_button = tk.Button(root, text="Browse", command=select_image)
browse_button.grid(row=0, column=1, padx=10, pady=10)

# Image preview label
image_label = Label(root)
image_label.grid(row=1, column=0, columnspan=2, padx=10, pady=10)

# Sliders for thresholds
slider1 = Scale(root, from_=0, to=255, orient=tk.HORIZONTAL, label="Threshold 1")
slider1.grid(row=2, column=0, padx=10, pady=5)
slider1.set(40)
slider1.bind("<Motion>", update_preview_image)

slider2 = Scale(root, from_=0, to=255, orient=tk.HORIZONTAL, label="Threshold 2")
slider2.grid(row=2, column=1, padx=10, pady=5)
slider2.set(120)
slider2.bind("<Motion>", update_preview_image)

slider3 = Scale(root, from_=0, to=255, orient=tk.HORIZONTAL, label="Threshold 3")
slider3.grid(row=3, column=0, padx=10, pady=5)
slider3.set(150)
slider3.bind("<Motion>", update_preview_image)

# Create and place the rotate button
rotate_button = Button(root, text="Rotate", command=rotate_image)
rotate_button.grid(row=3, column=1, padx=10, pady=10)

# Create and place the convert button
convert_button = Button(root, text="Convert and Save", command=convert_and_save)
convert_button.grid(row=4, column=1, padx=10, pady=10)

# Start the Tkinter event loop
root.mainloop()
update_preview_image()