# Module 3 – Intervention Strategy

## Overview

The Intervention Strategy module is responsible for selecting which user should receive a fact-checking intervention during the misinformation simulation.

The module provides multiple strategies for selecting an eligible user under a limited moderator budget.

The implemented strategies are:

1. Random Selection
2. Highest-Degree Selection
3. Marginal-Gain Selection

Each strategy follows the same common interface provided by the `InterventionStrategy` base class.

## Responsibilities

This module is responsible for:

- Identifying eligible users for intervention.
- Selecting an intervention target according to the chosen strategy.
- Providing multiple intervention strategies.
- Re-evaluating the current information state at each intervention opportunity.
- Returning the selected user to the Simulation Engine.

The module only selects a user. It does not directly modify the user's state or budget.

The Simulation Engine applies the selected intervention.

## Eligible Users

A user is eligible for intervention if they have not already been corrected.

Therefore:

- `Susceptible` → Eligible
- `Misinformed` → Eligible
- `Corrected` → Not eligible

A misinformed user can be selected for fact-checking to stop further misinformation propagation from that user.

## Intervention Strategies

### 1. Random Selection

The Random Selection strategy chooses an eligible user randomly.

It provides a simple baseline for comparison with more targeted strategies.

### 2. Highest-Degree Selection

The Highest-Degree strategy selects the eligible user with the largest number of connections in the social graph.

The degree represents the number of direct neighbours of a user.

If multiple eligible users have the same degree, the lower user ID is selected as the tie-breaker.

### 3. Marginal-Gain Selection

The Marginal-Gain strategy estimates the potential impact of intervening on each eligible user.

The current implementation uses a heuristic score based on:

- The number of directly connected susceptible users.
- Distinct susceptible users reachable within two hops.
- The configured misinformation probability.

The score is calculated as:

```text
Score =
Direct Susceptible Neighbours
+
pMisinfo × Two-Hop Susceptible Users
