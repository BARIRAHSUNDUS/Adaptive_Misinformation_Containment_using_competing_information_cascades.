# Adaptive Misinformation Containment Using Competing Information Cascades

## Project Overview

This project is a C++-based command-line simulation that models the
propagation of misinformation and fact-check information in a social
network represented as a graph.

The project investigates how limited fact-checking resources can be
allocated effectively by dynamically selecting intervention users based
on the current state of the network.

## Intervention Strategies

The system compares three strategies:

1. Random Selection
2. Highest-Degree Selection
3. Marginal-Gain-Based Selection

The marginal-gain strategy recalculates the expected benefit of each
possible intervention after the network state changes.

## Technologies

- C++
- Graphs and Adjacency Lists
- BFS
- Priority Queues
- Hash Maps
- Object-Oriented Programming
- GitHub

## Project Structure

- `include/` — Header files
- `src/` — C++ implementation files
- `tests/` — Test cases
- `data/` — Simulation input data
- `results/` — Simulation results
- `docs/` — Project documentation

## Team Development

The project is developed collaboratively using GitHub with feature
branches and pull requests.

## Expected Results

The simulation will compare the strategies based on:

- Misinformation reach
- Misinformation prevented
- Fact-check reach
- Execution time
