import { useState } from 'react';
import { Badge, Btn, Card, ProgressBar, SectionTitle } from '../components/ui';

const FirmwareUpdate = ({ connected }) => {
  const [updating, setUpdating] = useState(false);
  const [prog, setProg] = useState(0);

  const start = () => {
    setUpdating(true); setProg(0);
    let p = 0;
    const t = setInterval(()=>{
      p += Math.random()*4+1;
      if(p>=100){p=100;clearInterval(t);setUpdating(false);}
      setProg(p);
    }, 80);
  };

  const releases = [
    { v:"2.4.2", date:"2025-03-10", changes:["Fix SWD timing for STM32H7","Add QSPI support","Improve JTAG IR scan robustness"], current:false },
    { v:"2.4.1", date:"2025-02-20", changes:["Current firmware"], current:true },
    { v:"2.3.0", date:"2025-01-15", changes:["Added 1-Wire support","SPI improvements"], current:false },
  ];

  return (
    <div style={{padding:20, height:"100%", overflowY:"auto", display:"flex", flexDirection:"column", gap:16}}>
      <SectionTitle accent>RP2040 Firmware Manager</SectionTitle>
      <div style={{display:"grid", gridTemplateColumns:"1fr 1fr", gap:12}}>
        <Card glow={updating}>
          <SectionTitle accent>OTA Update</SectionTitle>
          {updating ? (
            <div style={{display:"flex",flexDirection:"column",gap:10}}>
              <div style={{fontFamily:"var(--font-mono)",fontSize:12,color:"var(--amber)"}}>Flashing firmware…</div>
              <ProgressBar value={prog} color="var(--amber)" animated />
              <div style={{fontFamily:"var(--font-mono)",fontSize:11,color:"var(--text-3)"}}>{prog.toFixed(0)}% complete</div>
            </div>
          ) : (
            <div style={{display:"flex",flexDirection:"column",gap:10}}>
              <div style={{fontFamily:"var(--font-mono)",fontSize:12,color:"var(--green)"}}>✓ Update available: v2.4.2</div>
              <Btn variant="amber" onClick={start} disabled={!connected} icon="↑">Flash v2.4.2 to Device</Btn>
              <div style={{fontSize:10,color:"var(--text-3)",fontFamily:"var(--font-mono)"}}>Device will reboot after flash. ~10 seconds.</div>
            </div>
          )}
        </Card>
        <Card>
          <SectionTitle accent>Current Device</SectionTitle>
          <div style={{fontFamily:"var(--font-mono)",fontSize:12,display:"flex",flexDirection:"column",gap:6}}>
            {[["Firmware","v2.4.1"],["Chip","RP2040 rev B2"],["Flash","2MB"],["SRAM","264KB"],["USB","HS OTG"]].map(([k,v])=>(
              <div key={k} style={{display:"flex",justifyContent:"space-between",borderBottom:"1px solid var(--border-dim)",paddingBottom:4}}>
                <span style={{color:"var(--text-3)"}}>{k}</span><span style={{color:"var(--text-1)"}}>{v}</span>
              </div>
            ))}
          </div>
        </Card>
      </div>
      <Card>
        <SectionTitle accent>Release History</SectionTitle>
        <div style={{display:"flex",flexDirection:"column",gap:8}}>
          {releases.map(r=>(
            <div key={r.v} style={{
              padding:"12px 14px", borderRadius:"var(--r-md)",
              border:`1px solid ${r.current?"var(--green)44":"var(--border-dim)"}`,
              background: r.current?"var(--green-glow)":"var(--bg-void)"
            }}>
              <div style={{display:"flex",gap:8,alignItems:"center",marginBottom:6}}>
                <span style={{fontFamily:"var(--font-mono)",fontSize:13,fontWeight:700,color:r.current?"var(--green)":"var(--text-1)"}}>v{r.v}</span>
                <span style={{fontFamily:"var(--font-mono)",fontSize:10,color:"var(--text-3)"}}>{r.date}</span>
                {r.current && <Badge small>Current</Badge>}
              </div>
              <div style={{display:"flex",flexDirection:"column",gap:2}}>
                {r.changes.map(c=>(
                  <span key={c} style={{fontFamily:"var(--font-mono)",fontSize:11,color:"var(--text-2)"}}>• {c}</span>
                ))}
              </div>
            </div>
          ))}
        </div>
      </Card>
    </div>
  );
};

export default FirmwareUpdate;
