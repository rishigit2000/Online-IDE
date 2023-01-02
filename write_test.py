import time 
from concurrent.futures import ThreadPoolExecutor , as_completed
import requests


def write_request(text):
    st = time.perf_counter()
    res = requests.get(f"http://localhost:18000/docs_writer?line_num=15&content={text}")
    en = time.perf_counter()
    # print(f"Writer took time = ", en-st)
    return en - st 


def main():
    texts = ["hello" , "my name" , "is tanmay" , "khabia" , "this is " , "my test bench"]
    print("Number of write requests", len(texts))
    st  = time.perf_counter()
    with ThreadPoolExecutor() as ex:
        time_list = ex.map(write_request , texts)
    en = time.perf_counter()
    print("total time taken " , en - st)
    for i  , val in enumerate(  time_list):
        print(f"time taken for thread {i} is: " , val)

if __name__=="__main__":
    main()
