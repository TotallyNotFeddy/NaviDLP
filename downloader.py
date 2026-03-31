# musich grabber systemes

# idle system 
from flask import Flask, request, Response
import requests

app = Flask(__name__) # shell

NAVIDROME = "http://localhost:4533" #defines local navidrome URL

@app.route("/rest/<path:endpoint>", methods=["GET", "POST"]) # asks for receive and send data
def proxy(endpoint): #defines proxy
    url = f"{NAVIDROME}/rest/{endpoint}"
    resp = requests.request(
        method=request.method,
        url=url,
        params=request.args,
        stream=True
    )
    return Response(resp.content, status=resp.status_code, content_type=resp.headers.get("content-type"))

if __name__ == "__main__":
    app.run(port=6969) # heheheh port name (for listening)

#search function
@app.route("/rest/search3")
def search():
    # grab the search query that substreamer sent
    query = request.args.get("query") # defines what query is
    
    # forward the same request to navidrome and get local results
    navidrome_resp = requests.get(
        f"{NAVIDROME}/rest/search3",
        params=request.args  # pass all the original parameters through
    )
    navidrome_results = navidrome_resp.json()
    
    # run yt-dlp search and capture the output
    ytdlp_proc = subprocess.run(
        ["yt-dlp", f"ytsearch5:{query}", "--dump-json", "--no-download"],
        capture_output=True,  # capture what yt-dlp prints
        text=True             # return it as a string not bytes
    )
    
    # yt-dlp returns one JSON object per line, parse each one
    ytdlp_results = [json.loads(line) for line in ytdlp_proc.stdout.strip().split("\n") if line]
    
    # map yt-dlp results into subsonic song format
    yt_songs = [{
        "id": f"yt-{r['id']}",        # prefix with yt- so we know it's not local
        "title": r.get("title", ""),
        "artist": r.get("uploader", ""),
        "duration": r.get("duration", 0),
        "coverArt": r.get("thumbnail", "")
    } for r in ytdlp_results]
    
    # grab local songs navidrome already found
    local_songs = navidrome_results.get("subsonic-response", {}).get("searchResult3", {}).get("song", [])
    
    # merge local + yt results
    merged = local_songs + yt_songs
    
    # return in subsonic format so substreamer understands it
    return {
        "subsonic-response": {
            "status": "ok",
            "version": "1.16.1",
            "searchResult3": {
                "song": merged
            }
        }
    }

#download function
@app.route("/rest/stream")
def download():
    # get the song id from the request
    song_id = request.args.get("id")
    
    # if id starts with "yt-" it's not downloaded yet
    if song_id.startswith("yt-"):
        # strip the "yt-" prefix to get the real youtube id
        yt_id = song_id.replace("yt-", "")
        
        # download the audio to navidrome music folder
        subprocess.run([
            "yt-dlp",
            f"https://www.youtube.com/watch?v={yt_id}",
            "-x",                          # extract audio only
            "--audio-format", "mp3",       # convert to mp3
            "--audio-quality", "0",        # best quality
            "--embed-metadata",            # embed title/artist tags
            "--embed-thumbnail",           # embed album art
            "-o", "/mnt/hdd/music/%(artist)s/%(title)s.%(ext)s"  # output path
        ])
        
        # tell navidrome to rescan so it finds the new file
        requests.get(f"{NAVIDROME}/rest/startScan", params=request.args)
        
        # wait a moment for navidrome to finish scanning
        import time
        time.sleep(3)
        
        # now forward the stream request to navidrome as normal
        resp = requests.get(f"{NAVIDROME}/rest/stream", params={
            **request.args,
            "id": yt_id  # use the real id now that it's in navidrome
        }, stream=True)
        
        return Response(resp.iter_content(chunk_size=1024), 
                       content_type=resp.headers.get("content-type"))
    
    # if it's a local song just forward straight to navidrome
    else:
        resp = requests.get(f"{NAVIDROME}/rest/stream", 
                           params=request.args, stream=True)
        return Response(resp.iter_content(chunk_size=1024),
                       content_type=resp.headers.get("content-type"))