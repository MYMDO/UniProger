import { useState } from 'react';
import { Badge, Btn, SectionTitle } from '../components/ui';
import { PROTOCOLS } from '../data/protocols';
import CHIP_DB from '../data/chips.json';

const ChipDatabase = ({ onSelectChip, selectedChip }) => {
  const [search, setSearch] = useState("");
  const [filterProto, setFilterProto] = useState("ALL");
  const [filterType, setFilterType] = useState("ALL");

  const filtered = CHIP_DB.filter(c => {
    const q = search.toLowerCase();
    const matchSearch = !q || c.name.toLowerCase().includes(q) || c.mfr.toLowerCase().includes(q) || c.type.toLowerCase().includes(q);
    const matchProto = filterProto === "ALL" || c.proto === filterProto;
    const matchType  = filterType  === "ALL" || c.type.includes(filterType);
    return matchSearch && matchProto && matchType;
  });

  const protos = ["ALL", ...Object.keys(PROTOCOLS)];
  const types  = ["ALL","SPI Flash","I2C EEPROM","MCU Flash","1-Wire","SPI EEPROM"];

  return (
    <div style={{padding:20, height:"100%", display:"flex", flexDirection:"column", gap:16}}>
      <SectionTitle accent>Chip Database — {CHIP_DB.length} Entries</SectionTitle>
      {/* Filters */}
      <div style={{display:"flex", gap:8, alignItems:"center"}}>
        <input
          className="input-field"
          style={{flex:1}}
          placeholder="Search by name, manufacturer, type…"
          value={search}
          onChange={e=>setSearch(e.target.value)}
        />
        <select value={filterProto} onChange={e=>setFilterProto(e.target.value)} className="input-field" style={{width:"auto", cursor:"pointer"}}>
          {protos.map(p => <option key={p} value={p}>{p}</option>)}
        </select>
        <select value={filterType} onChange={e=>setFilterType(e.target.value)} className="input-field" style={{width:"auto", cursor:"pointer"}}>
          {types.map(t => <option key={t} value={t}>{t}</option>)}
        </select>
        <span style={{fontFamily:"var(--font-mono)", fontSize:11, color:"var(--text-3)", flexShrink:0}}>
          {filtered.length} results
        </span>
      </div>
      {/* Table */}
      <div style={{overflowY:"auto", flex:1, borderRadius:"var(--r-lg)", border:"1px solid var(--border-dim)"}}>
        <table style={{width:"100%", borderCollapse:"collapse", fontFamily:"var(--font-mono)", fontSize:11}}>
          <thead>
            <tr style={{background:"var(--bg-base)", position:"sticky", top:0}}>
              {["Name","Manufacturer","Type","Size","Protocol","Voltage","Package","Operations","Est. Time",""].map(h => (
                <th key={h} style={{
                  padding:"10px 12px", textAlign:"left", color:"var(--text-3)",
                  fontWeight:600, letterSpacing:"0.08em", fontSize:10, textTransform:"uppercase",
                  borderBottom:"1px solid var(--border-dim)", whiteSpace:"nowrap"
                }}>{h}</th>
              ))}
            </tr>
          </thead>
          <tbody>
            {filtered.map((chip, i) => {
              const isSelected = selectedChip?.id === chip.id;
              const proto = PROTOCOLS[chip.proto];
              return (
                <tr key={chip.id} style={{
                  background: isSelected ? "var(--green-glow)" : (i%2===0?"var(--bg-surface)":"var(--bg-base)"),
                  borderBottom:"1px solid var(--border-dim)",
                  transition:"background 0.1s", cursor:"pointer",
                  outline: isSelected ? "1px solid var(--border)" : "none"
                }}
                onMouseEnter={e=>!isSelected&&(e.currentTarget.style.background="var(--bg-hover)")}
                onMouseLeave={e=>!isSelected&&(e.currentTarget.style.background=i%2===0?"var(--bg-surface)":"var(--bg-base)")}
                >
                  <td style={{padding:"9px 12px", color:"var(--green)", fontWeight:600}}>{chip.name}</td>
                  <td style={{padding:"9px 12px", color:"var(--text-2)"}}>{chip.mfr}</td>
                  <td style={{padding:"9px 12px", color:"var(--text-2)"}}>{chip.type}</td>
                  <td style={{padding:"9px 12px", color:"var(--text-1)"}}>{chip.size}</td>
                  <td style={{padding:"9px 12px"}}><Badge color={proto?.color} small>{chip.proto}</Badge></td>
                  <td style={{padding:"9px 12px", color:"var(--amber)"}}>{chip.voltage}</td>
                  <td style={{padding:"9px 12px", color:"var(--text-2)"}}>{chip.pkg}</td>
                  <td style={{padding:"9px 12px"}}>
                    <div style={{display:"flex", gap:3, flexWrap:"wrap"}}>
                      {chip.ops.map(op => <Badge key={op} small>{op}</Badge>)}
                    </div>
                  </td>
                  <td style={{padding:"9px 12px", color:"var(--text-3)"}}>{chip.time}</td>
                  <td style={{padding:"9px 12px"}}>
                    <Btn variant={isSelected?"primary":"ghost"} small onClick={()=>onSelectChip(chip)}>
                      {isSelected ? "✓ Selected" : "Select"}
                    </Btn>
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>
    </div>
  );
};

export default ChipDatabase;
