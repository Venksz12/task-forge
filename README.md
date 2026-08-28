# TaskForge
## Distributed Job Execution, Scheduling & Workflow Orchestration Platform

TaskForge is a C++23 distributed execution platform for an academic capstone and systems-engineering portfolio. It demonstrates a cohesive control plane rather than disconnected demos.

### Architecture

```text
Next.js Dashboard
       |
       v
C++23 REST Gateway (Boost.Beast/Asio)
       |
  +----+-------------+
  |    |             |
  v    v             v
Scheduler  Job API  Worker Manager
  |    |             |
  +----+-------------+
       |
       v
NATS JetStream ---> Worker Fleet
       |                |
       +-------+--------+
               v
       PostgreSQL / Redis
               |
       Prometheus / Grafana
```

### Engineering features
- C++23 concurrency using `std::jthread`, `std::stop_token`, mutexes, condition variables, atomics, futures and RAII.
- Weighted fair scheduling with HIGH/NORMAL/LOW weights 70/20/10 and a starvation bound.
- Kahn topological sorting, cycle detection, ready-node discovery and dependency failure propagation.
- At-least-once messaging with idempotent execution identity `(job_id, execution_id, attempt_number)`.
- Exponential retry backoff, cap, configurable maximum attempts and jitter injection.
- Redis TTL worker heartbeats with a 15-second liveness target and reassignment semantics.
- REST endpoints plus protobuf/gRPC contracts.
- PostgreSQL schema, constraints and operational indexes.
- Prometheus-compatible metrics, request IDs and structured JSON lifecycle logs.
- Docker multi-stage images, Compose and Kubernetes manifests.
- GoogleTest deterministic unit tests and k6 load testing.
- Responsive Next.js/TypeScript/Tailwind dashboard.

### Distributed-systems guarantees
TaskForge provides **at-least-once delivery**, not exactly-once processing. Duplicate deliveries are expected. The consumer idempotency boundary uses the execution identity and a unique database constraint. External side effects should use the same idempotency key.

### Scheduling algorithm
The scheduler owns FIFO queues per priority. Each scheduling round adds 70, 20 and 10 weighted deficit units. A non-empty queue with positive deficit is served and loses one unit. If any queue has waited longer than the starvation threshold, the oldest starving job is selected first. This prevents an unbounded high-priority stream from permanently starving lower priorities.

### DAG workflow algorithm
Workflow validation builds adjacency lists and indegrees. Kahn's algorithm repeatedly emits zero-indegree nodes and decrements dependents. If fewer nodes are emitted than exist, a cycle exists. Runtime states are PENDING, READY, RUNNING, SUCCEEDED, FAILED and BLOCKED. Independent ready branches can run concurrently.

### Retry strategy
`delay = min(max_delay, base_delay * 2^(attempt-1)) + jitter`, with the cap applied to the final value. The retry calculator is pure so tests do not sleep.

### Worker failure recovery
Workers publish TTL heartbeats to Redis. A manager marks stale workers unavailable, identifies their RUNNING leases, requeues eligible jobs and preserves attempt/idempotency history.

### REST API
- `GET /health`
- `GET /ready`
- `GET /metrics`
- `POST /api/v1/jobs`
- `GET /api/v1/jobs`
- `GET /api/v1/jobs/{id}`
- `POST /api/v1/jobs/{id}/cancel`
- `POST /api/v1/jobs/{id}/retry`
- `POST /api/v1/workflows`
- `GET /api/v1/workflows/{id}`
- `GET /api/v1/workers`
- `GET /api/v1/dlq`
- `POST /api/v1/dlq/{id}/retry`

Mutating endpoints use `Authorization: Bearer <token>`. Tokens are environment-configured; no production secret is embedded.

### Local development

```bash
docker compose -f infrastructure/docker-compose.yml up --build
```

Native build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DTASKFORGE_BUILD_TESTS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Frontend:

```bash
cd frontend
npm install
npm run dev
```

### Database
PostgreSQL 16 is the durable source of truth. UUIDs support distributed creation. The dispatch index `(status, priority, created_at)` supports queue-oriented scans. `job_attempts` has a unique `(job_id, execution_id, attempt_number)` constraint that closes the concurrent duplicate-execution race.

### Redis
Redis is used for TTL worker heartbeats, rate-limit counters, short-lived cache and coordination state. Heartbeat keys should expire automatically so crashed workers do not remain healthy forever.

### NATS JetStream
The intended stream is `TASKFORGE_JOBS`, with subject `taskforge.jobs`, durable consumers, explicit acknowledgements and redelivery. Workers ACK only after durable outcome recording.

### Observability
Prometheus scrapes `/metrics`. Grafana includes a starter dashboard for submitted/completed/failed jobs, queue depth and worker count. Logs are structured JSON and carry request/job/execution/worker identifiers where available.

### Kubernetes
Apply:

```bash
kubectl apply -f k8s/namespace.yaml
kubectl apply -f k8s/configmap.yaml
kubectl apply -f k8s/secrets.yaml
kubectl apply -f k8s/
```

The sample infrastructure uses single PostgreSQL/Redis/NATS instances for clarity. Gateway and worker deployments demonstrate rolling updates, probes, resources and HPA; the repository does not claim storage-layer high availability.

### Testing
GoogleTest covers DAG resolution, cycles, weighted scheduling, thread-pool completion/exception isolation and retry calculations. Integration scaffolding describes how to run dependency-backed tests with Compose.

### Load testing

```bash
k6 run tests/load/k6_benchmark.js
```

Thresholds cover `http_req_failed` and `http_req_duration`. No benchmark results are fabricated.

## Benchmark Results
Replace these fields only with measured values:
- Requests/sec: **MEASURED VALUE**
- p95 latency: **MEASURED VALUE**
- p99 latency: **MEASURED VALUE**
- Maximum concurrent workers: **MEASURED VALUE**
- Maximum sustainable queue depth: **MEASURED VALUE**

## Performance methodology
Warm the deployment first, then test increasing concurrency and payload sizes. Record throughput, p50/p95/p99 latency, CPU, memory, queue depth and worker utilization. Investigate database contention, scheduler locks, network latency, serialization overhead, worker saturation, Redis command rate and NATS throughput.

## Security considerations
Use environment variables/Kubernetes Secrets, validate request bodies, rate-limit clients and run containers as non-root where practical. Production hardening should add TLS, stronger identity, secret rotation, network policies and audit logging.

## Failure scenarios
- Worker crash: heartbeat expires, worker becomes unavailable, eligible jobs are requeued.
- NATS restart: durable stream/consumer state permits redelivery.
- PostgreSQL transient failure: request fails without claiming durable success.
- Duplicate delivery: idempotency identity prevents a second terminal execution.
- Retry exhaustion: job enters the DLQ.
- Gateway overload: rate limiter returns 429.

## Future improvements
Transactional outbox, stronger distributed leases, OpenTelemetry, multi-region placement, pluggable executors, persistent scheduler state and HA data-plane deployments.

## Resume Highlights
- Built a distributed job execution platform using C++23, PostgreSQL, Redis and NATS JetStream.
- Implemented weighted fair scheduling with starvation protection and FIFO ordering.
- Designed Kahn-based DAG orchestration with concurrent independent branches.
- Added at-least-once delivery, idempotent consumers, exponential retry and DLQ semantics.
- Implemented Redis TTL heartbeats and worker-failure reassignment.
- Exposed REST/gRPC contracts and Prometheus-compatible operational metrics.
- Containerized services with Docker Compose and Kubernetes probes, resources and HPA.
- Added deterministic GoogleTest coverage and k6 performance methodology.
