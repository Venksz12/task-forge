#!/usr/bin/env bash
set -euo pipefail
BASE=${TASKFORGE_URL:-http://localhost:8080}
TOKEN=${TASKFORGE_API_TOKEN:-dev-token}
curl -fsS "$BASE/health" | grep -q 'ok'
curl -fsS -H "Authorization: Bearer $TOKEN" "$BASE/api/v1/jobs" >/dev/null
curl -fsS -X POST -H "Authorization: Bearer $TOKEN" -H 'Content-Type: application/json' -d '{"name":"smoke","payload":{"kind":"noop"}}' "$BASE/api/v1/jobs" | grep -q job_id
echo 'TaskForge smoke test passed.'
