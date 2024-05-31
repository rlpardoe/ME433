from PIL import Image
from time import sleep
from picamera import PiCamera
from statistics import mean, median
from io import BytesIO
import serial


ser = serial.Serial(port='/dev/ttyS0', baudrate = 115200, timeout=1)
print("Serial open")
camera = PiCamera(resolution=(640, 480), framerate=30)
camera.color_effects = (128,128) # black and white image
camera.iso = 200;


def find_highest_brightness(image):
    width, height = image.size
    middle_row = height // 2
    row_pixels = image.load()
    max_brightnesses = [0]
    brightest_pixels = []
    for x in range(width):
        pixel = row_pixels[x, middle_row]
        #print(x, max_brightnesses)

        if pixel >= max_brightnesses[0]:
            if pixel >= 1.1*max_brightnesses[0]: # compare to lowest brightness of cohort
                max_brightnesses = [pixel]
                brightest_pixels = [x]
            else:
                max_brightnesses = max_brightnesses +[pixel]
                brightest_pixels = brightest_pixels + [x]
    return int(median(brightest_pixels))

# Main function for capturing grayscale images
#def capture_images(rate=8):
iters = 0
try:
    #with picamera.PiCamera() as camera:
      #  camera.resolution = (1024, 768)
      #  camera.color_effects = (128, 128)  # Set camera to capture in grayscale mode
        # Start capturing images continuously
    stream = BytesIO()
    for _ in camera.capture_continuous(stream, format='jpeg', use_video_port=True):
        iters +=1
        # Rewind the stream to the beginning
        stream.seek(0)
        # Load the image from the stream
        image = Image.open(stream).convert('L')  # Convert to grayscale
        # Find the pixel with the highest brightness in the middle row
        output = find_highest_brightness(image)

        #print("Brightest pixel in the middle row:", output)
        ser.write((str(output)+ '\r').encode())
        # Reset the stream for the next capture
        stream.seek(0)
        stream.truncate()
        # Pause to achieve the desired capture rate
        if iters >= 5000:
            break

finally:
    camera.close()
