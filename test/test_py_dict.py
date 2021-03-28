import time

def main():
    iter_c = 10_000


    before = time.time();

    hash_map = dict()

    for i in range(iter_c):
        hash_map[i] = i

    for i in hash_map.values():
        if i not in hash_map:
            print('fuck')
            break

    now = time.time()

    print('>',  now - before)



main()
