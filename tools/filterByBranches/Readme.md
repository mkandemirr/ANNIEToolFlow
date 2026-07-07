# filterByBranches

## Description

`filterByBranches` is a simple ROOT tree filtering tool. It reads an input ROOT file and keeps only the events where all selected branches have non-zero values.

The selected events are copied into a new ROOT file using `CloneTree(0)`, so the original tree structure and branches are preserved.

## Input

The tool reads its settings from a configuration file.

Example `config.ini`:

```ini
[input]
rootFile = input.root
treeName = Event

[output]
rootFile = filtered.root

[cuts]
branches = SimpleRecoFlag, SimpleRecoFV
```

## Selection Logic

For each event, the tool checks the branches listed in:

```ini
[cuts]
branches = branch1, branch2, branch3
```

An event is kept only if all listed branches are non-zero:

```text
branch1 != 0 AND branch2 != 0 AND branch3 != 0
```

If any listed branch is zero, the event is rejected.

## Output

The output ROOT file contains a filtered copy of the input tree.

The tool prints:

* total number of input events
* selected branch names
* number of events passing the filter
* number of output tree entries
* output file name

## Usage

```bash
./filterByBranches config.ini
```

## Notes

* Branch names in the config file should be separated by commas.
* Spaces around branch names are allowed.
* This tool is useful for quickly selecting events using existing boolean or integer flag branches.

