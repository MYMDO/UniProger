import { useState } from 'react';
import { Badge, Btn, SectionTitle } from '../components/ui';

function genHexRows(count = 32, seed = 0x41) {
  const rows = [];
  for (let r = 0; r < count; r++) {
    const bytes = Array.from({length:16}, (_, i) => ((seed + r * 16 + i) & 0xFF));
    const hex = bytes.map(b => b.toString(16).padStart(2,"0").toUpperCase());
    const ascii = bytes.map(b => (b >= 32 && b < 127) ? String.fromCharCode(b) : "·");
    rows.push({ hex, ascii });
  }
  return rows;
}

const HexEditor = ({ chip }) => {
  const [rows] = useState(() => genHexRows(32, 0x41));
  const [selected, setSelected] = useState(null);
  const [editMode, setEditMode] = useState(false);
  const [search, setSearch] = useState("");
  const [baseAddr] = useState(0);

  return (
    <div style={{padding:20, height:"100%", display:"flex", flexDirection:"column", gap:12, overflow:"hidden"}}>
      {/* Toolbar */}
      <div style={{display:"flex", gap:8, alignItems:"center", flexShrink:0}}>
        <SectionTitle accent>Hex Editor</SectionTitle>
        <div style={{flex:1}} />
        <input
          placeholder="Find hex bytes (e.g. FF A0 00)"
          value={search} onChange={e=>setSearch(e.target.value)}
          className="input-field" style={{width:240}}
        />
        <Btn variant="ghost" small onClick={()=>setEditMode(!editMode)}>
          {editMode?"✎ Edit ON":"✎ Edit OFF"}
        </Btn>
        <Btn variant="cyan" small>↓ Import</Btn>
        <Btn variant="amber" small>↑ Export</Btn>
        {chip && <Badge color="var(--green)">{chip.name}</Badge>}
      </div>

      {/* Hex view */}
      <div style={{flex:1, overflow:"auto", background:"var(--bg-void)", borderRadius:"var(--r-lg)", border:"1px solid var(--border-dim)"}}>
        <div style={{fontFamily:"var(--font-mono)", fontSize:12, minWidth:800}}>
          {/* Header */}
          <div style={{
            display:"grid", gridTemplateColumns:"100px repeat(16,30px) 1fr",
            padding:"8px 16px", background:"var(--bg-base)",
            borderBottom:"1px solid var(--border-dim)", position:"sticky", top:0
          }}>
            <span style={{color:"var(--text-3)", fontSize:10}}>OFFSET</span>
            {Array.from({length:16},(_,i)=>(
              <span key={i} style={{color:"var(--text-3)", fontSize:10, textAlign:"center"}}>
                {i.toString(16).toUpperCase().padStart(2,"0")}
              </span>
            ))}
            <span style={{color:"var(--text-3)", fontSize:10, paddingLeft:16}}>ASCII</span>
          </div>
          {rows.map((row, ri) => (
            <div key={ri} style={{
              display:"grid", gridTemplateColumns:"100px repeat(16,30px) 1fr",
              padding:"2px 16px", transition:"background 0.1s",
              background: ri%2===0?"transparent":"rgba(255,255,255,0.01)"
            }}
            onMouseEnter={e=>e.currentTarget.style.background="var(--bg-hover)"}
            onMouseLeave={e=>e.currentTarget.style.background=ri%2===0?"transparent":"rgba(255,255,255,0.01)"}
            >
              <span style={{color:"var(--text-3)", fontSize:11}}>
                {(baseAddr + ri*16).toString(16).padStart(8,"0").toUpperCase()}
              </span>
              {row.hex.map((b, bi) => {
                const isSel = selected === `${ri}-${bi}`;
                const isNull = b === "00";
                const isFF = b === "FF";
                return (
                  <span key={bi} onClick={()=>setSelected(`${ri}-${bi}`)} style={{
                    textAlign:"center", cursor:"pointer", borderRadius:2, padding:"1px 0",
                    color: isSel ? "var(--bg-void)" : isNull ? "var(--text-3)" : isFF ? "var(--red-dim)" : "var(--text-1)",
                    background: isSel ? "var(--green)" : "transparent",
                    fontSize:11, transition:"all 0.1s"
                  }}>{b}</span>
                );
              })}
              <span style={{paddingLeft:16, color:"var(--text-3)", letterSpacing:"0.05em", fontSize:11}}>
                {row.ascii.join("")}
              </span>
            </div>
          ))}
        </div>
      </div>

      {/* Status bar */}
      <div style={{
        display:"flex", gap:16, padding:"6px 12px",
        background:"var(--bg-base)", border:"1px solid var(--border-dim)",
        borderRadius:"var(--r-md)", fontFamily:"var(--font-mono)", fontSize:10, color:"var(--text-3)", flexShrink:0
      }}>
        {selected && (() => {
          const [r,b] = selected.split("-").map(Number);
          const byte = parseInt(rows[r]?.hex[b]||"00",16);
          return <>
            <span>Offset: <b style={{color:"var(--green)"}}>0x{(r*16+b).toString(16).padStart(8,"0").toUpperCase()}</b></span>
            <span>Dec: <b style={{color:"var(--cyan)"}}>{byte}</b></span>
            <span>Hex: <b style={{color:"var(--amber)"}}>0x{byte.toString(16).toUpperCase().padStart(2,"0")}</b></span>
            <span>Bin: <b style={{color:"var(--purple)"}}>{byte.toString(2).padStart(8,"0")}</b></span>
          </>;
        })()}
        <span style={{marginLeft:"auto"}}>
          Size: {(rows.length*16).toString(16).toUpperCase()} bytes · {editMode ? "EDIT MODE" : "READ-ONLY"}
        </span>
      </div>
    </div>
  );
};

export default HexEditor;
