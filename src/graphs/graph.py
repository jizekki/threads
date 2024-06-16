import os
import matplotlib.pyplot as plt
import re
import subprocess
import sys

max_threads = 500
max_yields = 50
nb_executions = 10
exec_time = 30


def file_not_found_message(path: str) -> str:
    return "File " + os.path.abspath(path) + " does not exist"


def get_time_exec(string: str) -> int:
    """
    Returns the execution time of the test program
    :param string: what is normally displayed on the prompt by subprocess.check_output() on the path of the test
    :return:
    """
    line_list = string.splitlines()
    for line in line_list:
        if re.search("[0-9]+ us", line)!= None:
            lineTime = line
    word_list = lineTime.split()
    return int(word_list[-2])


def graph_create(path: str):
    x_values = [i for i in range(1, max_threads)]
    y_threads = []

    for n in x_values:
        t = 0
        for k in range(nb_executions):
            out = subprocess.check_output(path + " " + str(n), shell=True).decode(sys.stdout.encoding)
            t += get_time_exec(out)
        y_threads.append(t / nb_executions)
    return x_values, y_threads


def graph_switch(path: str):
    x_threads = [i for i in range(1, max_threads)]
    x_yields = [i for i in range(1, max_yields)]
    y_threads = [] * max_threads
    y_yields = [] * max_yields

    for n in x_threads:
        t = 0
        for k in range(nb_executions):
            out = subprocess.check_output(path + " " + str(n) + " 10", shell=True).decode(sys.stdout.encoding)
            t += get_time_exec(out)
        y_threads.append(t / nb_executions)

    for n in x_yields:
        t = 0
        for k in range(nb_executions):
            out = subprocess.check_output(path + " 10 " + str(n), shell=True).decode(sys.stdout.encoding)
            t += get_time_exec(out)
        y_yields.append(t / nb_executions)
    return x_threads, y_threads, x_yields, y_yields


def graph_fibo(path: str):
    x_values = [3]
    y_values = []
    t = 0
    while t < exec_time:
        t = 0
        for k in range(nb_executions):
            out = subprocess.check_output(path + " " + str(x_values[-1]), shell=True)
            line_list = out.splitlines()
            word_list = line_list[-1].split()
            t_string = word_list[-2]
            t_tab = t_string.split(b'e')
            t += float(t_tab[0]) * 10 ** int(t_tab[1])
        t = t / nb_executions
        y_values.append(t)
        x_values.append(x_values[-1] + 1)
    x_values.pop()
    return x_values, y_values


def graph_compare(path: str, save_path: str) -> None:
    if re.search("/2[0-9]+-", path) is not None:
        pthread_path = path + "-pthread"
        assert os.path.isfile(path), file_not_found_message(path)
        assert os.path.isfile(pthread_path), file_not_found_message(pthread_path)
        x, y = graph_create(path)
        x_pthread, y_pthread = graph_create(pthread_path)

        plt.plot(x, y, color='b', label="thread library")
        plt.plot(x_pthread, y_pthread, color='r', label="pthread")
        plt.xlabel("Number of threads")
        plt.ylabel("Execution time (us)")
        plt.title(save_path.split('/')[-1].split('.')[0])
        plt.legend()
        plt.savefig(save_path)

    elif re.search("/3[0-9]+-", path) is not None:
        pthread_path = path + "-pthread"
        assert os.path.isfile(path), file_not_found_message(path)
        assert os.path.isfile(pthread_path), file_not_found_message(pthread_path)
        x_thread, y_thread, x_yield, y_yield = graph_switch(path)
        x_thread_pthread, y_thread_pthread, x_yield_pthread, y_yield_pthread = graph_switch(pthread_path)

        fig, (threadAxis, yieldAxis) = plt.subplots(2, 1, constrained_layout=True)
        plt.title(save_path.split('/')[-1].split('.')[0])
        threadAxis.plot(x_thread, y_thread, label="Thread library")
        threadAxis.plot(x_thread_pthread, y_thread_pthread, label="pthread")
        yieldAxis.plot(x_yield, y_yield, label="Thread library")
        yieldAxis.plot(x_yield_pthread, y_yield_pthread, label="pthread")

        threadAxis.set_xlabel("number of threads")
        yieldAxis.set_xlabel("number of yields")
        threadAxis.set_ylabel("execution time (µs)")
        yieldAxis.set_ylabel("execution time (µs)")

        plt.legend()
        plt.savefig(save_path)

    elif re.search("/51-", path) is not None:
        pthread_path = path + "-pthread"
        assert os.path.isfile(path), file_not_found_message(path)
        assert os.path.isfile(pthread_path), file_not_found_message(pthread_path)
        x, y = graph_fibo(path)
        x_pthread, y_pthread = graph_fibo(pthread_path)
        plt.plot(x, y, color='b', label="Thread library")
        plt.plot(x_pthread, y_pthread, color='r', label="pthread")
        plt.xlabel("value")
        plt.ylabel("Execution time (s)")
        plt.title(save_path.split('/')[-1].split('.')[0])
        plt.legend()
        plt.savefig(save_path)

    elif re.search("/53-", path) is not None:
        pthread_path = path + "-pthread"
        assert os.path.isfile(path), file_not_found_message(path)
        assert os.path.isfile(pthread_path), file_not_found_message(pthread_path)
        x, y = graph_create(path)
        x_pthread, y_pthread = graph_create(pthread_path)

        plt.plot(x, y, color='b', label="thread library")
        plt.plot(x_pthread, y_pthread, color='r', label="pthread")
        plt.xlabel("Array size")
        plt.ylabel("Execution time (us)")
        plt.title(save_path.split('/')[-1].split('.')[0])
        plt.legend()
        plt.savefig(save_path)

    else:
        print("Graph not supported for the file " + os.path.abspath(path))


def main():
    if len(sys.argv) == 1:
        print("USAGE: <executable> [output_path]")
        exit(1)
    if len(sys.argv) == 2:
        save_path = sys.argv[1].split("/")[-1] + ".png"
    else:
        save_path = sys.argv[2]
    graph_compare(sys.argv[1], save_path)


if __name__ == "__main__":
    main()

