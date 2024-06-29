#import time
import requests

HOST = "localhost"
PORT = 12345
url = f"http://{HOST}:{PORT}"

def send_request(request):
    #print(f"Sending request: {request}")
    
    #start_time = time.time_ns() // 1000000
    response = requests.get(url)
    #end_time = time.time_ns() // 1000000
    
    # # # # # # # # # # # # # # # # # #
    assert(response.status_code == 200)
    # # # # # # # # # # # # # # # # # #
    
    #print(end_time-start_time, "ms") 

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) == 1:
        NUM_REQUESTS = 1
    else:
        NUM_REQUESTS = int(sys.argv[1])
    
    #start_time = time.time_ns() // 1000000
    
    for i in range(1, NUM_REQUESTS + 1):
        send_request(f"REQUEST_{i}")
    
    #end_time = time.time_ns() // 1000000
    
    #print("----------------------------------------")
    #print(f"Time elapsed: {end_time - start_time} ms")
    #print(f"Avg time elapsed per request: {(end_time - start_time) / NUM_REQUESTS} ms")
    #print("----------------------------------------")

