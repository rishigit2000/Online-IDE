import time
import requests
import random
from concurrent.futures import ThreadPoolExecutor, as_completed


def make_request(thread_id):
    line_num = random.randrange(1, 50)
    local_start = time.perf_counter()
    # print(f"local {thread_id} start time = ", local_start)
    response = requests.get(f"http://localhost:18000/reader?line_num={line_num}")
    if response.status_code != 200:
        print("ERROR OCCURED!")
    local_end = time.perf_counter()
    # print(f"local {thread_id} end time = ", local_end)
    # print(f"Reader thread {thread_id} took time = ", local_end-local_start) #use for reader-writer tests
    return local_end - local_start

def main():
    start = time.perf_counter()
    thread_id = 1
    with ThreadPoolExecutor() as executor:
        results = [executor.submit(make_request, id) for id in range(1, 1000)]

        for f in as_completed(results):
            print(f"Thread {thread_id} execution time taken = ", f.result())
            thread_id += 1

    end = time.perf_counter()

    print("total time taken ", end - start)

if __name__=="__main__":
    main()
