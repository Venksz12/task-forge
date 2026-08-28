import Link from 'next/link';
export default function Page(){return <main className="p-8"><div className="mx-auto max-w-6xl"><Link href="/" className="text-cyan-400">← Dashboard</Link><h1 className="mt-6 text-3xl font-bold">Jobs</h1><div className="card mt-6 text-slate-400">Recent executions, filters, attempts, cancellation and retry.</div></div></main>;}
