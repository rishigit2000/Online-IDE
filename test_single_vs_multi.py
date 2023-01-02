import time
import requests
from concurrent.futures import ThreadPoolExecutor, as_completed

# In web_client.c, change the THREAD_POOL_SIZE to 1 to test single vs multi.


def make_request():
    local_start = time.perf_counter()
    print("local start time = ", local_start)
    response = requests.get(f"http://localhost:18000/index.html")
    if response.status_code != 200:
        print("ERROR OCCURED!")
    local_end = time.perf_counter()
    print("local end time = ", local_end)
    return local_end - local_start


start = time.perf_counter()
idx = 1
with ThreadPoolExecutor() as executor:
    results = [executor.submit(make_request) for _ in range(1000)]

    for f in as_completed(results):
        print(f"Thread {idx} execution time taken = ", f.result())
        idx += 1

end = time.perf_counter()

print("total time taken ", end - start)
