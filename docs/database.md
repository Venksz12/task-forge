# Database
The `jobs` dispatch index supports status/priority/time queue scans. `job_attempts` has a unique `(job_id, execution_id, attempt_number)` key to close concurrent duplicate races. Worker, workflow, event and DLQ indexes match operational lookup patterns.
