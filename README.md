# Adaptive Misinformation Containment Using Competing Information Cascades

A C++ graph-based simulation that models misinformation propagation and compares fact-checking intervention strategies under limited moderator resources.

## Project Overview

This project models a social network as a graph, where users are represented as nodes and their connections as edges. The simulation models the spread of misinformation and fact-check information over successive time steps.

A limited moderator budget is used to perform fact-checking interventions. The project compares three intervention strategies:

- Random Selection
- Highest-Degree Selection
- Marginal-Gain-Based Selection

The marginal-gain strategy evaluates the expected reduction in misinformation for possible interventions based on the current state of the network and recalculates the decision after each intervention.

## Technologies

- C++
- C++ Standard Library
- Graphs and Adjacency Lists
- BFS
- Priority Queues
- Hash Maps
- Object-Oriented Programming
- GitHub

## Project Goals

- Model misinformation propagation in a social network.
- Simulate fact-checking interventions under a limited budget.
- Dynamically select interventions based on marginal gain.
- Compare intervention strategies under the same conditions.
- Analyse misinformation spread, misinformation prevented, fact-check reach, and runtime.
