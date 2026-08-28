# TaskForge ( Distributed Job Execution & Workflow Engine)

TaskForge is a distributed job execution and workflow orchestration platform developed using **C++23, PostgreSQL, Redis, NATS JetStream, and Next.js**. The project demonstrates practical distributed-systems concepts including concurrent job execution, priority scheduling, DAG-based workflow orchestration, fault tolerance, retries, worker failure recovery, idempotency, observability, and containerized deployment.

## Architecture

```text
                         ┌──────────────────────┐
                         │      Next.js UI      │
                         │ TypeScript + Tailwind│
                         └──────────┬───────────┘
                                    │ REST
                                    ▼
                         ┌──────────────────────┐
                         │    C++23 Gateway     │
                         │ Boost.Asio / Beast   │
                         │ Auth + Rate Limiting │
                         └──────────┬───────────┘
                                    │
              ┌─────────────────────┼─────────────────────┐
              ▼                     ▼                     ▼
       ┌─────────────┐      ┌─────────────┐      ┌─────────────┐
       │  Scheduler  │      │  Workflow   │      │    Worker   │
       │ Fair Queue  │      │ DAG Engine  │      │   Runtime   │
       └──────┬──────┘      └──────┬──────┘      └──────┬──────┘
              │                    │                     │
              └────────────────────┼─────────────────────┘
                                   ▼
                         ┌──────────────────────┐
                         │   NATS JetStream     │
                         │ Durable Job Events   │
                         └──────────┬───────────┘
                                    │
                   ┌────────────────┼────────────────┐
                   ▼                ▼                ▼
             PostgreSQL           Redis          Prometheus
             Durable State     Heartbeats/TTL      Metrics
                                                     │
                                                  Grafana
```

TaskForge separates request handling, scheduling, workflow coordination and job execution while using PostgreSQL for durable state, Redis for ephemeral coordination and worker liveness, and NATS JetStream for durable asynchronous messaging.

## How TaskForge Works

### 1. Job Submission

A client submits a job through the REST API exposed by the C++23 Gateway.

The Gateway validates the request, authenticates the client, applies rate limiting and forwards the job into the execution pipeline.

A job progresses through states such as:

```text
SUBMITTED → QUEUED → RUNNING → SUCCEEDED
                         │
                         ▼
                       FAILED
                         │
                         ▼
                      RETRYING
                         │
                   retry exhausted
                         ▼
                    DEAD_LETTER
```

Each job maintains execution metadata including priority, timeout, attempt number, worker assignment and timestamps.

### 2. Priority Scheduling

The Scheduler maintains multiple priority queues and uses **weighted fair scheduling** to prevent high-priority workloads from permanently starving lower-priority jobs.

A simplified scheduling policy is:

```text
HIGH       → 70%
NORMAL     → 20%
LOW        → 10%
```

Within each priority level, FIFO ordering is preserved where possible. The scheduler also tracks queue depth and execution statistics for operational monitoring.

### 3. Workflow Orchestration

TaskForge supports workflows represented as **Directed Acyclic Graphs (DAGs)**.

For example:

```text
        ┌──► Validate ──► Transform ──► Store
        │
Start ──┤
        │
        └──► Enrich ───────────────────► Store
```

Workflow dependencies are resolved using **Kahn's topological sorting algorithm**. The workflow engine detects cycles, determines which nodes are ready to execute, tracks dependency completion and propagates failures through the workflow.

Independent tasks can execute concurrently when sufficient workers are available.

### 4. Distributed Job Execution

The Scheduler publishes executable jobs through **NATS JetStream**. Workers consume jobs using durable consumers and explicit acknowledgements.

The messaging model follows **at-least-once delivery semantics**. A worker acknowledges a message only after the execution result has been durably recorded.

This means duplicate delivery is possible by design, so TaskForge uses idempotency mechanisms to prevent duplicate logical execution.

### 5. Idempotent Execution

Each execution is associated with an execution identity consisting of the job, execution and attempt information.

This allows the system to distinguish between:

* a new execution,
* a retry,
* a redelivered message,
* and a duplicate delivery.

Idempotency is particularly important when a worker completes a task but fails before acknowledging the corresponding NATS message.

### 6. Retry and Dead Letter Queue

Failed jobs can be retried using **exponential backoff with jitter**.

```text
Attempt 1
   │
   └── failure
        │
        ▼
     backoff
        │
Attempt 2
   │
   └── failure
        │
        ▼
     backoff
        │
       ...
        │
        ▼
   retry limit reached
        │
        ▼
     DEAD_LETTER
```

Every attempt is recorded so that execution history can be inspected. Jobs that exhaust their retry policy are moved to the **Dead Letter Queue (DLQ)**.

### 7. Worker Heartbeats and Failure Recovery

Workers periodically publish heartbeat information using Redis.

Redis TTL keys provide an ephemeral liveness mechanism:

```text
Worker
   │
   │ heartbeat
   ▼
 Redis TTL
   │
   ├── alive → worker considered healthy
   │
   └── TTL expires
          │
          ▼
   worker considered unavailable
          │
          ▼
   running jobs reassigned
```

This allows the scheduler to identify failed workers and recover jobs that were assigned to unavailable workers.

## Technology Stack

| Component     | Technology                   |
| ------------- | ---------------------------- |
| Backend       | **C++23**                    |
| Networking    | **Boost.Asio / Boost.Beast** |
| RPC           | **gRPC + Protocol Buffers**  |
| Database      | **PostgreSQL 16**            |
| Messaging     | **NATS JetStream**           |
| Coordination  | **Redis 7**                  |
| Frontend      | **Next.js + TypeScript**     |
| UI            | **Tailwind CSS**             |
| Charts        | **Recharts**                 |
| Testing       | **GoogleTest**               |
| Load Testing  | **k6**                       |
| Metrics       | **Prometheus**               |
| Dashboards    | **Grafana**                  |
| Containers    | **Docker / Docker Compose**  |
| Orchestration | **Kubernetes**               |
| CI/CD         | **Jenkins**                  |
| Build         | **CMake**                    |

## C++23 Design

The backend uses modern C++23 features and concurrency primitives including:

* `std::jthread`
* `std::stop_token`
* `std::atomic`
* `std::mutex`
* `std::condition_variable`
* RAII
* smart pointers
* scoped locking
* move semantics
* `std::optional`
* `std::variant`
* `std::chrono`

These are used to build thread-safe worker pools, scheduling components and service infrastructure while maintaining clear ownership and resource-management semantics.

## Data Persistence

PostgreSQL acts as the durable system of record.

The database contains entities such as:

```text
users
jobs
job_attempts
workflows
workflow_nodes
workflow_dependencies
workers
job_events
dead_letter_jobs
```

Foreign keys, unique constraints, status constraints, timestamps and indexes are used to maintain data integrity and support execution-history and operational queries.

Redis is intentionally used for short-lived coordination data such as worker heartbeats, TTL-based liveness and rate limiting rather than as the primary source of durable job state.

## API

TaskForge provides REST and gRPC interfaces.

Representative REST endpoints include:

```text
POST   /api/v1/jobs
GET    /api/v1/jobs
GET    /api/v1/jobs/{id}
POST   /api/v1/jobs/{id}/cancel
POST   /api/v1/jobs/{id}/retry

POST   /api/v1/workflows
GET    /api/v1/workflows/{id}

GET    /api/v1/workers
GET    /api/v1/dlq

GET    /health
GET    /ready
GET    /metrics
```

The API layer is responsible for request validation, authentication, authorization, rate limiting and propagation of request/execution identifiers.

## Observability

TaskForge includes an observability layer using **Prometheus and Grafana**.

Metrics cover areas such as:

* job throughput
* job success/failure
* retry counts
* worker count
* queue depth
* worker heartbeat age
* job reassignment
* HTTP request latency
* HTTP error rates

Structured logging and identifiers such as request ID, job ID, execution ID and worker ID make distributed operations easier to diagnose.

## Web Dashboard

The Next.js dashboard provides an operational view of the platform.

It includes views for:

* System health
* Jobs
* Job attempts
* Workflows
* DAG execution
* Workers
* Queue activity
* Dead Letter Queue
* Performance metrics

The dashboard communicates with the backend APIs and presents execution and infrastructure information in a single interface.

## Deployment

TaskForge is designed to run using Docker Compose for local development and Kubernetes for container orchestration.

The development environment includes:

```text
Gateway
Scheduler
Worker
Frontend
PostgreSQL
Redis
NATS
Prometheus
Grafana
```

Kubernetes manifests provide deployments, services, health probes, resource configuration and horizontal scaling configuration.

## Testing

The project includes automated tests for important system components, including:

* DAG construction and cycle detection
* Topological ordering
* Scheduling fairness
* Retry and backoff logic
* Thread-pool concurrency
* Job execution behavior
* Integration-level service behavior

A **k6** load-testing suite is also included for evaluating API and job-submission performance under controlled workloads.

Performance claims should be based on measurements obtained from the target execution environment rather than predetermined benchmark numbers.

## Distributed Systems Properties

TaskForge focuses on several practical distributed-systems concepts:

* **At-least-once message delivery**
* **Idempotent job execution**
* **Fault detection**
* **Worker failure recovery**
* **Durable job state**
* **Retry and backoff**
* **Dead Letter Queue processing**
* **Priority-aware scheduling**
* **Starvation prevention**
* **DAG dependency management**
* **Concurrent task execution**
* **Distributed coordination**
* **Observability**

The system does not rely on exactly-once message delivery. Instead, it assumes duplicate delivery can occur and uses durable state and idempotency to make processing safe.

## Project Structure

```text
TaskForge/
├── CMakeLists.txt
├── Makefile
├── README.md
│
├── apps/
│   ├── gateway/
│   ├── scheduler/
│   └── worker/
│
├── include/
│   └── taskforge/
│
├── src/
│   ├── common/
│   ├── db/
│   ├── messaging/
│   ├── scheduler/
│   ├── workflow/
│   └── worker/
│
├── proto/
├── database/
├── frontend/
├── tests/
├── loadtests/
├── docker/
├── k8s/
├── monitoring/
└── docs/
```

## Building

TaskForge uses CMake as its primary build system.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

For local development, Docker Compose can be used to start the supporting infrastructure and application services.

```bash
docker compose up --build
```

Refer to the project documentation for environment configuration, database initialization, service startup and Kubernetes deployment procedures.
