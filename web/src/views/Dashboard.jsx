import { useState } from 'react';
import { Badge, Btn, Card, SectionTitle } from '../components/ui';
import { PROTOCOLS } from '../data/protocols';
import CHIP_DB from '../data/chips.json';

const Dashboard = ({ connected, onConnect, chipSelected, onSelectChip }) => {
  const [autoDetecting, setAutoDetecting] = useState(false);
  const [detected, setDetected] = useState(null);

  const runAutoDetect = () => {
    setAutoDetecting(true);
    setDetected(null);
    setTimeout(() => { setDetected(CHIP_DB[0]); setAutoDetecting(false); }, 2200);
  };

  const stats = [
    { label:"Supported Chips",   val:"4,847",  sub:"+127 this month", color:"var(--green)" },
    { label:"Protocols",         val:"12",     sub:"SPI/I2C/JTAG/SWD/…", color:"var(--cyan)" },
    { label:"Operations Today",  val:"31",     sub:"28 OK · 3 ERR", color:"var(--amber)" },
    { label:"Server Latency",    val:"2ms",    sub:"WebSocket active", color:"var(--purple)" },
  ];

  return (
    <div style={{padding:24, overflowY:"auto", height:"100%", display:"flex", flexDirection:"column", gap:20}}>
      {/* Hero banner */}
      {!connected && (
        <Card style={{
          background:"linear-gradient(135deg,#0d1a14 0%,#0a1520 100%)",
          border:"1px solid var(--border)", padding:28, position:"relative", overflow:"hidden"
        }} glow>
          <div style={{
            position:"absolute", top:-40, right:-40, width:200, height:200,
            borderRadius:"50%", background:"var(--green-glow)", filter:"blur(60px)"
          }} />
          <div style={{fontFamily:"var(--font-display)", fontSize:22, fontWeight:800, color:"var(--green)", marginBottom:6}}>
            Universal Programmer · Analyzer
          </div>
          <div style={{color:"var(--text-2)", fontSize:13, marginBottom:20, maxWidth:460}}>
            RP2040-based protocol bridge. Connect your device via USB, select a chip from the database
            of 4,847 entries, and start programming in seconds.
          </div>
          <div style={{display:"flex", gap:8, flexWrap:"wrap"}}>
            {Object.entries(PROTOCOLS).map(([name,{color,icon}]) => (
              <Badge key={name} color={color}>{icon} {name}</Badge>
            ))}
          </div>
          <div style={{marginTop:20}}>
            <Btn variant="primary" onClick={onConnect} icon="⚡" style={{fontSize:13, padding:"10px 24px"}}>
              Connect RP2040 Device
            </Btn>
          </div>
        </Card>
      )}

      {/* Stats row */}
      <div style={{display:"grid", gridTemplateColumns:"repeat(4,1fr)", gap:12}}>
        {stats.map(s => (
          <Card key={s.label} style={{padding:14}}>
            <div style={{fontFamily:"var(--font-mono)", fontSize:22, fontWeight:700, color:s.color, lineHeight:1}}>
              {s.val}
            </div>
            <div style={{fontSize:10, color:"var(--text-2)", marginTop:4, fontWeight:600, letterSpacing:"0.06em", textTransform:"uppercase"}}>{s.label}</div>
            <div style={{fontSize:10, color:"var(--text-3)", marginTop:2}}>{s.sub}</div>
          </Card>
        ))}
      </div>

      {/* Auto-detect + selected chip */}
      <div style={{display:"grid", gridTemplateColumns:"1fr 1fr", gap:12}}>
        <Card>
          <SectionTitle accent>Auto Detection</SectionTitle>
          <div style={{display:"flex", flexDirection:"column", gap:12}}>
            <div style={{
              background:"var(--bg-void)", borderRadius:"var(--r-md)", padding:14,
              border:"1px solid var(--border-dim)", fontFamily:"var(--font-mono)", fontSize:11
            }}>
              {autoDetecting ? (
                <div style={{display:"flex", alignItems:"center", gap:8, color:"var(--amber)"}}>
                  <span style={{animation:"spin 0.8s linear infinite", display:"inline-block"}}>⟳</span>
                  Scanning JEDEC ID · probing protocols…
                </div>
              ) : detected ? (
                <div style={{display:"flex", flexDirection:"column", gap:6}}>
                  <div style={{color:"var(--green)"}}>✓ Chip detected!</div>
                  <div style={{color:"var(--text-1)", fontSize:14, fontWeight:700}}>{detected.name}</div>
                  <div style={{color:"var(--text-2)"}}>Mfr: {detected.mfr} · {detected.type} · {detected.size}</div>
                  <div style={{display:"flex",gap:6,marginTop:4}}>
                    <Badge color={PROTOCOLS[detected.proto]?.color}>{detected.proto}</Badge>
                    <Badge color="var(--text-3)">{detected.voltage}</Badge>
                    <Badge color="var(--text-3)">{detected.pkg}</Badge>
                  </div>
                </div>
              ) : (
                <div style={{color:"var(--text-3)"}}>No chip detected. Connect device and run scan.</div>
              )}
            </div>
            <Btn variant={autoDetecting?"ghost":"primary"} onClick={runAutoDetect} disabled={!connected || autoDetecting} icon="⊛">
              {autoDetecting ? "Scanning…" : "Auto-Detect Chip"}
            </Btn>
            {detected && <Btn variant="cyan" onClick={() => onSelectChip(detected)} icon="→">Use Detected Chip</Btn>}
          </div>
        </Card>

        <Card>
          <SectionTitle accent>Selected Chip</SectionTitle>
          {chipSelected ? (
            <div style={{display:"flex", flexDirection:"column", gap:8}}>
              <div style={{fontFamily:"var(--font-mono)", fontSize:16, fontWeight:700, color:"var(--green)"}}>{chipSelected.name}</div>
              <div style={{color:"var(--text-2)", fontSize:12}}>{chipSelected.mfr} · {chipSelected.type}</div>
              {[
                ["Size",    chipSelected.size],
                ["Protocol",chipSelected.proto],
                ["Voltage", chipSelected.voltage],
                ["Package", chipSelected.pkg],
                ["Est. Time",chipSelected.time],
              ].map(([k,v]) => (
                <div key={k} style={{display:"flex", justifyContent:"space-between", fontSize:11, borderBottom:"1px solid var(--border-dim)", paddingBottom:4}}>
                  <span style={{color:"var(--text-3)", fontFamily:"var(--font-mono)"}}>{k}</span>
                  <span style={{color:"var(--text-1)", fontFamily:"var(--font-mono)"}}>{v}</span>
                </div>
              ))}
              <div style={{display:"flex", gap:6, flexWrap:"wrap", marginTop:4}}>
                {chipSelected.ops.map(op => <Badge key={op} small>{op}</Badge>)}
              </div>
            </div>
          ) : (
            <div style={{color:"var(--text-3)", fontSize:12, fontFamily:"var(--font-mono)"}}>
              No chip selected. Use auto-detect or browse the database.
            </div>
          )}
        </Card>
      </div>

      {/* Protocol map */}
      <Card>
        <SectionTitle accent>Supported Protocols</SectionTitle>
        <div style={{display:"grid", gridTemplateColumns:"repeat(4,1fr)", gap:8}}>
          {Object.entries(PROTOCOLS).map(([name, {color, icon, pins}]) => (
            <div key={name} style={{
              background:"var(--bg-void)", border:`1px solid ${color}22`,
              borderRadius:"var(--r-md)", padding:"12px",
              transition:"all 0.15s", cursor:"default"
            }}>
              <div style={{display:"flex", alignItems:"center", gap:8, marginBottom:6}}>
                <span style={{fontSize:18, color}}>{icon}</span>
                <span style={{fontFamily:"var(--font-mono)", fontSize:12, fontWeight:700, color}}>{name}</span>
              </div>
              <div style={{display:"flex", gap:4, flexWrap:"wrap"}}>
                {pins.map(p => <Badge key={p} color={color} small>{p}</Badge>)}
              </div>
            </div>
          ))}
        </div>
      </Card>
    </div>
  );
};

export default Dashboard;
