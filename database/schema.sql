CREATE EXTENSION IF NOT EXISTS pgcrypto;
CREATE TABLE IF NOT EXISTS users(id uuid PRIMARY KEY DEFAULT gen_random_uuid(),email text NOT NULL UNIQUE,created_at timestamptz NOT NULL DEFAULT now());
CREATE TABLE IF NOT EXISTS jobs(
 id uuid PRIMARY KEY DEFAULT gen_random_uuid(), execution_id uuid NOT NULL DEFAULT gen_random_uuid(),
 name text NOT NULL CHECK(length(name) BETWEEN 1 AND 200),
 priority text NOT NULL CHECK(priority IN ('HIGH','NORMAL','LOW')),
 status text NOT NULL CHECK(status IN ('SUBMITTED','QUEUED','RUNNING','SUCCEEDED','FAILED','RETRYING','DEAD_LETTER','CANCELLED')),
 payload jsonb NOT NULL, timeout_ms bigint NOT NULL CHECK(timeout_ms>0), attempt integer NOT NULL DEFAULT 0 CHECK(attempt>=0),
 max_attempts integer NOT NULL DEFAULT 4 CHECK(max_attempts>0), worker_id text, created_at timestamptz NOT NULL DEFAULT now(), updated_at timestamptz NOT NULL DEFAULT now());
CREATE INDEX IF NOT EXISTS idx_jobs_dispatch ON jobs(status,priority,created_at);
CREATE INDEX IF NOT EXISTS idx_jobs_worker ON jobs(worker_id,status);
CREATE TABLE IF NOT EXISTS job_attempts(
 id uuid PRIMARY KEY DEFAULT gen_random_uuid(),job_id uuid NOT NULL REFERENCES jobs(id) ON DELETE CASCADE,
 execution_id uuid NOT NULL,attempt_number integer NOT NULL,worker_id text,status text NOT NULL,
 started_at timestamptz,finished_at timestamptz,error_code text,error_message text,
 UNIQUE(job_id,execution_id,attempt_number));
CREATE INDEX IF NOT EXISTS idx_attempts_worker_status ON job_attempts(worker_id,status);
CREATE INDEX IF NOT EXISTS idx_attempts_job ON job_attempts(job_id,attempt_number DESC);
CREATE TABLE IF NOT EXISTS workflows(id uuid PRIMARY KEY DEFAULT gen_random_uuid(),name text NOT NULL,status text NOT NULL CHECK(status IN ('PENDING','RUNNING','SUCCEEDED','FAILED')),created_at timestamptz NOT NULL DEFAULT now(),updated_at timestamptz NOT NULL DEFAULT now());
CREATE TABLE IF NOT EXISTS workflow_nodes(id uuid PRIMARY KEY DEFAULT gen_random_uuid(),workflow_id uuid NOT NULL REFERENCES workflows(id) ON DELETE CASCADE,node_key text NOT NULL,job_id uuid REFERENCES jobs(id),status text NOT NULL CHECK(status IN ('PENDING','READY','RUNNING','SUCCEEDED','FAILED','BLOCKED')),UNIQUE(workflow_id,node_key));
CREATE TABLE IF NOT EXISTS workflow_dependencies(workflow_id uuid NOT NULL REFERENCES workflows(id) ON DELETE CASCADE,from_node_id uuid NOT NULL REFERENCES workflow_nodes(id) ON DELETE CASCADE,to_node_id uuid NOT NULL REFERENCES workflow_nodes(id) ON DELETE CASCADE,PRIMARY KEY(workflow_id,from_node_id,to_node_id),CHECK(from_node_id<>to_node_id));
CREATE INDEX IF NOT EXISTS idx_workflow_nodes_status ON workflow_nodes(workflow_id,status);
CREATE TABLE IF NOT EXISTS workers(id text PRIMARY KEY,hostname text NOT NULL,status text NOT NULL,capacity integer NOT NULL CHECK(capacity>0),active_jobs integer NOT NULL DEFAULT 0,registered_at timestamptz NOT NULL DEFAULT now(),last_heartbeat timestamptz NOT NULL DEFAULT now(),version text NOT NULL,healthy boolean NOT NULL DEFAULT false);
CREATE TABLE IF NOT EXISTS job_events(id uuid PRIMARY KEY DEFAULT gen_random_uuid(),job_id uuid NOT NULL REFERENCES jobs(id) ON DELETE CASCADE,event_type text NOT NULL,payload jsonb NOT NULL DEFAULT '{}',occurred_at timestamptz NOT NULL DEFAULT now());
CREATE INDEX IF NOT EXISTS idx_job_events_job_time ON job_events(job_id,occurred_at DESC);
CREATE TABLE IF NOT EXISTS dead_letter_jobs(id uuid PRIMARY KEY DEFAULT gen_random_uuid(),job_id uuid NOT NULL UNIQUE REFERENCES jobs(id) ON DELETE CASCADE,reason text NOT NULL,attempt_count integer NOT NULL,original_created_at timestamptz NOT NULL,moved_at timestamptz NOT NULL DEFAULT now());
CREATE INDEX IF NOT EXISTS idx_dlq_moved ON dead_letter_jobs(moved_at DESC);

CREATE OR REPLACE FUNCTION taskforge_claim_job(p_job uuid, p_worker text)
RETURNS boolean LANGUAGE plpgsql AS $$
BEGIN
  UPDATE jobs SET status='RUNNING', worker_id=p_worker, attempt=attempt+1, updated_at=now()
  WHERE id=p_job AND status='QUEUED';
  RETURN FOUND;
END $$;

CREATE OR REPLACE FUNCTION taskforge_requeue_stale_jobs(p_cutoff timestamptz)
RETURNS integer LANGUAGE plpgsql AS $$
DECLARE n integer;
BEGIN
  UPDATE jobs j SET status='RETRYING', worker_id=NULL, updated_at=now()
  WHERE j.status='RUNNING' AND j.worker_id IN
    (SELECT w.id FROM workers w WHERE w.last_heartbeat < p_cutoff AND w.healthy=false);
  GET DIAGNOSTICS n = ROW_COUNT;
  UPDATE jobs SET status='QUEUED', updated_at=now() WHERE status='RETRYING';
  RETURN n;
END $$;
