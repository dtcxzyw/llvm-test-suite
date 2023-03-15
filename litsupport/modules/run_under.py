"""Test module to run benchmarks in a wrapper application. This is typically
used to prefix the benchmark command with simulator commands."""
from litsupport import shellcommand
from litsupport import testplan
import re


def mutateCommandLine(context, commandline):
    timefile = context.tmpBase + ".inst"
    context.instfiles.append(timefile)

    cmd = shellcommand.parse(commandline)
    run_under_cmd = shellcommand.parse(context.config.run_under)

    if run_under_cmd.stdin is not None or \
       run_under_cmd.stdout is not None or \
       run_under_cmd.stderr is not None or \
       run_under_cmd.workdir is not None or \
       run_under_cmd.envvars:
        raise Exception("invalid run_under argument!")

    cmd.wrap(run_under_cmd.executable, run_under_cmd.arguments+[timefile])

    return cmd.toCommandline()


def getUserTime(filename):
    """Extract the user time from a .time file produced by qemu"""
    with open(filename) as fd:
        contents = fd.read()
        return getUserTimeFromContents(contents)


def getUserTimeFromContents(contents):
    def from_bytes(s): return s.decode("utf-8") if type(s) == bytes else s
    lines = [from_bytes(l) for l in contents.splitlines()]
    line = [line for line in lines if line.startswith('insns')]
    assert len(line) == 1

    m = re.match(r'insns:\s+([0-9]+)', line[0])
    return float(m.group(1)) / 1_000_000_000  # 1G Hz


def _collectTime(context, instfiles, metric_name='exec_time'):
    time = 0.0
    for timefile in instfiles:
        filecontent = context.read_result_file(context, timefile)
        time += getUserTimeFromContents(filecontent)
    return {metric_name: time}


def mutatePlan(context, plan):
    run_under = context.config.run_under
    if run_under:
        if not hasattr(context, "instfiles"):
            context.instfiles = []
        plan.runscript = testplan.mutateScript(context, plan.runscript,
                                               mutateCommandLine)
        plan.metric_collectors.append(
            lambda context: _collectTime(context, context.instfiles)
        )
