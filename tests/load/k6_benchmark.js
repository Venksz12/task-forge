import http from 'k6/http';
import { check } from 'k6';
export const options={vus:20,duration:'30s',thresholds:{http_req_failed:['rate<0.01'],http_req_duration:['p(95)<500']}};
export default function(){const base=__ENV.TASKFORGE_URL||'http://localhost:8080';const token=__ENV.TASKFORGE_API_TOKEN||'dev-token';const r=http.post(`${base}/api/v1/jobs`,JSON.stringify({name:`k6-${__VU}-${__ITER}`,priority:'NORMAL',timeout_ms:30000,payload:{command:'benchmark'}}),{headers:{'Content-Type':'application/json',Authorization:`Bearer ${token}`}});check(r,{'accepted':x=>x.status===202});}
