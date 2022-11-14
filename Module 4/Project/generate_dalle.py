import openai
import urllib.request

def generate_image(inter, prompt, file_name):
    openai.api_key = "API_KEY"
    response = openai.Image.create(
        prompt=prompt,
        n=1,
        size="1024x1024"
    )

    image_url = response['data'][0]['url']
    urllib.request.urlretrieve(image_url, file_name)
