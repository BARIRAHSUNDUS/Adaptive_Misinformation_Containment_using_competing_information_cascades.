# Misinformation vs Fact-Check Simulator

A C++17 simulation of competing information cascades on a social network, where misinformation and fact-check information spread probabilistically under a limited moderator budget.

## Overview

Misinformation can spread rapidly through social networks. However, moderators have limited resources and cannot fact-check every user. This project simulates the spread of misinformation and fact-checking to determine which users should be prioritized for intervention.

The project compares different intervention strategies and uses **Marginal Gain** as its primary strategy to estimate which intervention can reduce future misinformation the most.

## Key Features

- Graph-based social network using an adjacency list
- Probabilistic misinformation and fact-check propagation
- Tick-based simulation with dynamic user states
- Limited moderator intervention budget
- Random, Highest Degree, and Marginal Gain strategies
- Monte Carlo simulation for estimating intervention effectiveness
- Priority queue for candidate selection
- Benchmarking and reproducible experiments
- Command-line interface

## How It Works

1. A social network is generated using the specified number of users and connection probability.
2. Misinformation starts from a selected source user and spreads probabilistically.
3. The moderator intervenes at scheduled intervals using the selected strategy.
4. Corrected users spread fact-check information through the network.
5. The simulation tracks the spread of misinformation and fact-checking over discrete ticks.
6. Different strategies are compared using simulation metrics.

## Intervention Strategies

- **Random:** Selects an eligible user randomly.
- **Highest Degree:** Selects the eligible user with the most direct connections.
- **Marginal Gain:** Estimates the expected reduction in future misinformation for each candidate using Monte Carlo simulations and selects the candidate with the highest estimated gain.
- **No Intervention:** Acts as a baseline for comparing the effectiveness of the other strategies.

Marginal Gain is an adaptive greedy strategy. Candidate scores are recalculated as the simulation state changes.

## DSA and OOP Concepts

**Data Structures and Algorithms**
- Graphs and adjacency lists
- Graph traversal
- Queues for cascade propagation
- Priority queues for candidate ranking
- Monte Carlo simulation and greedy selection

**Object-Oriented Programming**
- Classes and encapsulation
- Inheritance and polymorphism through intervention strategies
- Modular design and separation of responsibilities

## Technology Stack

- **Language:** C++17
- **Build System:** Make
- **Interface:** Command-line interface
- **Testing:** Automated test suite






## Benchmarking

Benchmark mode compares intervention strategies under the same experimental conditions across multiple runs.

The main metrics include:
- Misinformation reach
- Final number of misinformed users
- Fact-check reach
- Misinformation prevented
- Execution time

A fixed random seed can be used to improve reproducibility.

## Team Contributions

## Team Contributions

| Module | Responsibility | Team Member |
|---|---|---|
| Social Graph | Graph creation, adjacency list, degree calculation, and graph operations |Vani Aggarwal |
| Cascade Simulation | Misinformation and fact-check propagation, user states, and tick-based updates |Avni Singh|
| Intervention Module | Intervention strategies, Monte Carlo evaluation, and candidate selection | Barirah Sundus |
| Simulation Engine | Module integration, intervention scheduling, CLI, metrics, and benchmarking |Shagun|

- Develop a graphical user interface.
- Improve the efficiency of computationally expensive simulations.
- Expand benchmarking across different network configurations.
- Add visualizations for simulation results.
