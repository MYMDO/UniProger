import { useState, useRef, useEffect } from 'react';
import { Badge, Btn, Card, ProgressBar, SectionTitle } from '../components/ui';
import { PROTOCOLS, LOG_COLORS } from '../data/protocols';

const OPS = [
  { id:"read",   label:"Read",   icon:"↓", color:"var(--cyan)",  desc:"Read chip contents to buffer" },
  { id:"write",  label:"Write",  icon:"↑", color:"var(--green)", desc:"Write buffer to chip" },
  { id:"erase",  label:"Erase",  icon:"⊠", color:"var(--red)",   desc:"Full chip erase" },
  { id:"verify", label:"Verify", icon:"✓", color:"var(--amber)", desc:"Compare chip vs buffer" },
];

const Operations = ({ chip, connected }) => {
  const [operation, setOperation] = useState(null);
  const [progress, setProgress] = useState(0);
  const [speed, setSpeed] = useState(0);
  const [elapsed, setElapsed] = useState(0);
  const [logs, setLogs] = useState([]);
  const [done, setDone] = useState(false);
  const [address, setAddress] = useState("00000000");
  const [length, setLength] = useState("01000000");
  const timerRef = useRef(null);
  const logsRef = useRef(null);

  const startOp = (op) => {
    setOperation(op); setProgress(0); setSpeed(0); setElapsed(0); setDone(false);
    setLogs([
      { t:"now", level:"INFO", msg:`Starting ${op.toUpperCase()} · addr:0x${address} len:0x${length}` },
      { t:"now", level:"TX",   msg:`CMD: 0x${op==='erase'?'C7':'03'} → RP2040 bridge` },
    ]);
    let p = 0;
    timerRef.current = setInterval(() => {
      p += Math.random() * 3 + 1;
      if (p >= 100) { p = 100; clearInterval(timerRef.current); setDone(true);
        setLogs(l => [...l, { t:"now", level:"OK", msg:`${op.toUpperCase()} completed · CRC32: 0xA3F4C21B` }]);
      }
      setProgress(p);
      setSpeed(Math.floor(Math.random()*200+800));
      setElapsed(e=>e+0.1);
      if (Math.random() > 0.8) {
        const msgs = [
          `Block 0x${Math.floor(p*1000).toString(16).padStart(6,"0")} ✓`,
          `RX: ${Array.from({length:8},()=>Math.floor(Math.random()*256).toString(16).padStart(2,"0")).join(" ")}`,
          `Verify sector ${Math.floor(p/4)} OK`,
        ];
        setLogs(l => [...l, { t:"now", level: p<100?"RX":"OK", msg: msgs[Math.floor(Math.random()*msgs.length)] }].slice(-60));
      }
    }, 100);
  };

  useEffect(() => { if (logsRef.current) logsRef.current.scrollTop = logsRef.current.scrollHeight; }, [logs]);

  if (!chip) return (
    <div style={{display:"flex", alignItems:"center", justifyContent:"center", height:"100%", flexDirection:"column", gap:12}}>
      <div style={{fontSize:36, color:"var(--text-3)"}}>⊞</div>
      <div style={{color:"var(--text-3)", fontFamily:"var(--font-mono)", fontSize:13}}>No chip selected. Go to Chip Database.</div>
    </div>
  );

  const opActive = operation && !done;

  return (
    <div style={{padding:20, height:"100%", display:"flex", gap:16, overflow:"hidden"}}>
      {/* Left panel */}
      <div style={{width:280, display:"flex", flexDirection:"column", gap:12, flexShrink:0}}>
        <Card glow>
          <SectionTitle accent>Target Chip</SectionTitle>
          <div style={{fontFamily:"var(--font-mono)", fontSize:16, fontWeight:700, color:"var(--green)"}}>{chip.name}</div>
          <div style={{fontSize:11, color:"var(--text-2)", marginTop:2}}>{chip.mfr} · {chip.size} · {chip.voltage}</div>
          <div style={{marginTop:8}}><Badge color={PROTOCOLS[chip.proto]?.color}>{chip.proto}</Badge></div>
        </Card>
        <Card>
          <SectionTitle accent>Address Range</SectionTitle>
          <div style={{display:"flex", flexDirection:"column", gap:8}}>
            {[["Start Address",address,setAddress],["Length",length,setLength]].map(([l,v,set])=>(
              <div key={l}>
                <div style={{fontSize:10, color:"var(--text-3)", marginBottom:3, fontFamily:"var(--font-mono)", letterSpacing:"0.08em"}}>{l}</div>
                <div style={{display:"flex", alignItems:"center"}}>
                  <span style={{fontFamily:"var(--font-mono)", fontSize:12, color:"var(--text-3)", marginRight:4}}>0x</span>
                  <input className="input-field" value={v} onChange={e=>set(e.target.value.replace(/[^0-9A-Fa-f]/g,"").slice(0,8))} disabled={opActive} />
                </div>
              </div>
            ))}
          </div>
        </Card>
        <Card>
          <SectionTitle accent>Operations</SectionTitle>
          <div style={{display:"flex", flexDirection:"column", gap:6}}>
            {OPS.filter(o => chip.ops.includes(o.id)).map(op => (
              <button key={op.id} onClick={() => !opActive && startOp(op.id)} disabled={opActive || !connected}
                style={{
                  display:"flex", alignItems:"center", gap:10, padding:"10px 12px",
                  borderRadius:"var(--r-md)", border:`1px solid ${operation===op.id?op.color+"88":"var(--border-dim)"}`,
                  background: operation===op.id ? `${op.color}15` : "var(--bg-void)",
                  cursor: opActive||!connected ? "not-allowed" : "pointer", textAlign:"left",
                  transition:"all 0.15s", opacity: !connected ? 0.4 : 1
                }}>
                <span style={{fontSize:18, color:op.color}}>{op.icon}</span>
                <div>
                  <div style={{fontFamily:"var(--font-ui)", fontSize:12, fontWeight:600, color:op.color}}>{op.label}</div>
                  <div style={{fontFamily:"var(--font-mono)", fontSize:10, color:"var(--text-3)"}}>{op.desc}</div>
                </div>
              </button>
            ))}
          </div>
        </Card>
      </div>
      {/* Right panel */}
      <div style={{flex:1, display:"flex", flexDirection:"column", gap:12, overflow:"hidden"}}>
        <Card style={{flexShrink:0}}>
          <SectionTitle accent>Progress</SectionTitle>
          {operation ? (
            <div style={{display:"flex", flexDirection:"column", gap:10}}>
              <div style={{display:"flex", justifyContent:"space-between", alignItems:"center"}}>
                <span style={{fontFamily:"var(--font-mono)", fontSize:12, color:"var(--text-2)"}}>{operation.toUpperCase()} · {chip.name}</span>
                <span style={{fontFamily:"var(--font-mono)", fontSize:14, fontWeight:700, color: done?"var(--green)":"var(--text-1)"}}>{progress.toFixed(1)}%</span>
              </div>
              <ProgressBar value={progress} color={done?"var(--green)":OPS.find(o=>o.id===operation)?.color} animated={opActive} />
              <div style={{display:"flex", gap:20, fontFamily:"var(--font-mono)", fontSize:11, color:"var(--text-3)"}}>
                <span>Speed: <span style={{color:"var(--cyan)"}}>{speed} KB/s</span></span>
                <span>Elapsed: <span style={{color:"var(--text-1)"}}>{elapsed.toFixed(1)}s</span></span>
                <span>Remaining: <span style={{color:"var(--amber)"}}>{done?"0.0s":((100-progress)/progress*elapsed).toFixed(1)+"s"}</span></span>
                {done && <span style={{color:"var(--green)", fontWeight:700}}>✓ COMPLETE</span>}
              </div>
            </div>
          ) : (
            <div style={{color:"var(--text-3)", fontFamily:"var(--font-mono)", fontSize:12}}>Select an operation to begin.</div>
          )}
        </Card>
        <Card style={{flex:1, overflow:"hidden", display:"flex", flexDirection:"column"}}>
          <div style={{display:"flex", justifyContent:"space-between", alignItems:"center", marginBottom:10}}>
            <SectionTitle accent>Operation Log</SectionTitle>
            <Btn variant="ghost" small onClick={()=>setLogs([])}>Clear</Btn>
          </div>
          <div ref={logsRef} style={{
            flex:1, overflowY:"auto", fontFamily:"var(--font-mono)", fontSize:11, lineHeight:1.7,
            background:"var(--bg-void)", borderRadius:"var(--r-md)", padding:"10px 12px",
            border:"1px solid var(--border-dim)"
          }}>
            {logs.map((log, i) => (
              <div key={i} style={{display:"flex", gap:10}}>
                <span style={{color:"var(--text-3)", flexShrink:0}}>›</span>
                <span style={{color:LOG_COLORS[log.level]||"var(--text-2)"}}>{log.msg}</span>
              </div>
            ))}
            {logs.length === 0 && <span style={{color:"var(--text-3)"}}>Waiting for operation…</span>}
          </div>
        </Card>
      </div>
    </div>
  );
};

export default Operations;
