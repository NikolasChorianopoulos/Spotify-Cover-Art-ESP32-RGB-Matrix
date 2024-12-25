import requests
import json
from flask import Flask, request, redirect
import webbrowser

# Spotify API credentials
client_id = "e57a0662b41a486ea2cdcfe94f457876"
client_secret = "c8feb5189c2747eaa1327d74c8a9bf2c"
redirect_uri = "http://localhost:8080/callback"
scope = "user-read-playback-state user-read-currently-playing"

app = Flask(__name__)

# Authorization URL
auth_url = "https://accounts.spotify.com/authorize"
token_url = "https://accounts.spotify.com/api/token"

# Step 1: Redirect the user to the Spotify authorization page
def authorize_user():
    params = {
        "client_id": client_id,
        "response_type": "code",
        "redirect_uri": redirect_uri,
        "scope": scope
    }
    
    # Open the authorization URL in the user's browser
    auth_request = requests.Request('GET', auth_url, params=params).prepare()
    webbrowser.open(auth_request.url)

# Step 2: Handle the redirect and extract the authorization code
@app.route('/callback')
def callback():
    auth_code = request.args.get("code")

    # Step 3: Exchange the authorization code for access and refresh tokens
    token_data = {
        "grant_type": "authorization_code",
        "code": auth_code,
        "redirect_uri": redirect_uri,
        "client_id": client_id,
        "client_secret": client_secret
    }

    # Request tokens from Spotify
    response = requests.post(token_url, data=token_data)
    tokens = response.json()

    # Display the access token and refresh token
    access_token = tokens.get("access_token")
    refresh_token = tokens.get("refresh_token")

    return f"Access Token: {access_token}<br>Refresh Token: {refresh_token}"

if __name__ == '__main__':
    # Step 1: Initiate the authorization process
    authorize_user()
    
    # Start Flask server to listen for the Spotify redirect
    app.run(port=8080)
