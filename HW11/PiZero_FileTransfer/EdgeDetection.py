from PIL import Image
from time import sleep
from picamera import PiCamera
from statistics import mean, median
from io import BytesIO

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

        print("Brightest pixel in the middle row:", output)
        # Reset the stream for the next capture
        stream.seek(0)
        stream.truncate()
        # Pause to achieve the desired capture rate
        if iters >= 16:
            break

finally:
    camera.close()

# plot the data for debugging
#import matplotlib.pyplot as plt
#plt.plot(index,reds,'r-', index,greens,'g-', index,blues,'b-',index,sum,'k-',index,sum_smooth,'r-',index, diff_sum,'y-*')
#plt.show()

'''
# open the image
img_data = Image.open("line_low_res.jpg")
img_data = img_data.convert('RGB')

# get the image size
width, height = img_data.size
print("w: "+ str(width)+" h: "+str(height))

# set some filtering IIR parameters
alpha = 0.95
beta = 0.05

# loop through every pixel in every row to find the line
for row in range(height):
    index = []
    reds = []
    greens = []
    blues = []
    sum = [] # the brightness of a pixel
    sum_smooth = [] # the smoothed out brightness of a pixel
    diff_sum = [] # the derivative of the brightness of a pixel
    for i in range(width):
        r, g, b = img_data.getpixel((i, row)) # get the raw color of a pixel
        index.append(i)
        reds.append(r)
        greens.append(g)
        blues.append(b)
        sum.append(r+g+b) # calculate the brightness
        if i == 0:
            sum_smooth.append(sum[-1])
            diff_sum.append(0)
        else:
            sum_smooth.append(sum_smooth[-1]*alpha+sum[-1]*beta) # smooth out the brightness with IIR
            diff_sum.append(sum_smooth[-1] - sum_smooth[-2]) # the derivative

    # find the edges, where the derivative is most positive and negative
    posleft = diff_sum.index(max(diff_sum))
    posright = diff_sum.index(min(diff_sum))
    #print("r: " + str(row) + " l: = "+str(posleft)+" r: "+str(posright))

    # draw green dots where we think the edge is
    img_data.putpixel((posleft,row),(0,255,0))
    img_data.putpixel((posright,row),(0,255,0))
    # draw a red dot where we thing the center of the line is
    img_data.putpixel((int(posleft+(posright-posleft)/2),row),(255,0,0))

# show the image with where we think the edges and center are
img_data.show()
'''