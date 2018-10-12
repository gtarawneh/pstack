#!/usr/bin/env python

import json
import redis
import random
import docopt
import datetime
import subprocess

from Queue import Queue
from threading import Thread

from psim import psim
from simple_redis import pop_json
from simple_redis import push_json


usage="""POETS Daemon (PD) v0.1

Usage:
  pd.py [options]

Options:
  -w --workers=<n>   Specify number of workers (core count by default).
  -n --name=<name>   Specify engine name (hostname by default)
  -r --redis=<host>  Specify Redis host [default: localhost:6379].
  -q --quiet         Suppress all outputs.

"""


def log(msg):
    now = datetime.datetime.utcnow()
    dt_str = now.strftime("%y-%m-%d %H:%M")
    print "%s - %s" % (dt_str, msg)


def parse_connection_str(con_str):
    """Parse connection string in the form host:port."""
    try:
        host_str, port_str = con_str.split(':')
        return (host_str, int(port_str))
    except ValueError:
        raise Exception("Could not parse '%s'" % con_str)


def wait_jobs(redis_cl, nworkers=1):

    queue = Queue()

    def create_worker(index):
        return Thread(target=run_worker, args=(redis_cl, queue, index))

    workers = map(create_worker, range(nworkers))

    for worker in workers:
        worker.setDaemon(True)
        worker.start()

    while True:
        try:
            item = queue.get(timeout=1)
            queue.task_done()
            log("[Worker %d] %s" % item)
        except Exception:
            pass
        except KeyboardInterrupt:
            break


def run_worker(redis_cl, queue, index):
    """Wait for and process jobs from Redis "jobs" queue."""

    def queue_msg(msg):
        queue.put((index, msg))

    queue_msg("Waiting for jobs ...")

    while True:
        job = pop_json(redis_cl, "jobs")
        queue_msg("Running %(name)s (region %(region)s) ..." % job)
        result = psim(
            xml=job["xml"],
            region_map=job["region_map"],
            regions=[job["region"]],
            options={"debug": False, "level": 0},
            quiet=True,
            force_socat=True)
        push_json(redis_cl, job["result_queue"], result)
        queue_msg("Completed" )


def get_capabilities(name=None, nworkers=None):
    """Get hostname and core count using 'hostname' and 'nproc'.

    Inputs:
      - name     (str): an *override value*
      - nworkers (str): an *override value*

    Returns:
      - (name, workers) tuple of type (str, int)

    These are obtained by running the tools 'hostname' and 'nproc'.
    """

    def run_command(cmd):
        """Run command and return output, or None if command fails."""
        run = subprocess.check_output
        try:
            return run(cmd, shell=True, stderr=subprocess.PIPE).strip()
        except Exception:
            return None

    def get_rand_name():
        return "unnamed-%s" % "".join(random.sample("0123456789", 6))

    name = name or run_command("hostname") or get_rand_name()
    nworkers = int(nworkers or run_command("nproc") or "1")

    return (name, nworkers)


def register_engine(redis_cl, name, nworkers):
    """Register engine.

    Stores engine name and capabilities using client name as key.
    """

    engine_information = {
        "name": name,
        "type": "Simulator (psim)",
        "_nresources": nworkers,
        "resources": "%d threads" % nworkers if nworkers>1 else "1 thread"
    }

    redis_cl.client_setname(name)
    redis_cl.set(name, json.dumps(engine_information))


def main():
    args = docopt.docopt(usage, version="v0.1")
    host, port = parse_connection_str(args["--redis"])
    redis_cl = redis.StrictRedis(host, port)
    name, nworkers = get_capabilities(args["--name"], args["--workers"])
    register_engine(redis_cl, name, nworkers)
    log("Starting (Engine %s)..." % name)
    wait_jobs(redis_cl, nworkers)
    log("Shutting down ...")


if __name__ == '__main__':
    main()
