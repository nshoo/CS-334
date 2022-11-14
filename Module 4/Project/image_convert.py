from linedraw import image_to_json

def convert(file_name):
    image_to_json(file_name, resolution=512, draw_contours=2, draw_hatch=16)
