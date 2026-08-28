# Architecture
Gateway handles HTTP/authentication/rate-limit concerns. Scheduler owns dispatch order. Workers own execution. PostgreSQL is durable state, Redis is ephemeral liveness/coordination, and NATS JetStream is durable asynchronous delivery.

Delivery is at-least-once. Workers acknowledge messages only after durable outcome recording. Duplicate messages are suppressed at the execution identity boundary. The architecture deliberately avoids claiming exactly-once processing.
