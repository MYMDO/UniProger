import { useState } from 'react';
import { Badge, Btn, Card, Dot, SectionTitle } from '../components/ui';
import { PROTOCOLS } from '../data/protocols';

const MOCK_TRANSACTIONS = [
  { id:1, t:"00:00.001", dir:"TX", proto:"SPI", cmd:"CMD_JEDEC_ID",    data:"9F",                resp:"EF 40 18", status:"OK",   us:2  },
  { id:2, t:"00:00.003", dir:"TX", proto:"SPI", cmd:"CMD_READ_STATUS", data:"05",                resp:"00",       status:"OK",   us:1  },
  { id:3, t:"00:00.005", dir:"TX", proto:"SPI", cmd:"CMD_WRITE_EN",    data:"06",                resp:"—",        status:"OK",   us:1  },
  { id:4, t:"00:00.008", dir:"TX", proto:"SPI", cmd:"CMD_SECTOR_ERASE",data:"20 00 00 00",       resp:"—",        status:"OK",   us:45 },
  { id:5, t:"00:00.053", dir:"RX", proto:"SPI", cmd:"STATUS_POLL",     data:"05",                resp:"01",       status:"BUSY", us:1  },
  { id:6, t:"00:00.100", dir:"RX", proto:"SPI", cmd:"STATUS_POLL",     data:"05",                resp:"00",       status:"OK",   us:1  },
  { id:7, t:"00:00.101", dir:"TX", proto:"SPI", cmd:"CMD_PAGE_PROG",   data:"02 00 00 00 + 256B",resp:"—",        status:"OK",   us:3  },
  { id:8, t:"00:00.104", dir:"TX", proto:"SPI", cmd:"CMD_READ_DATA",   data:"03 00 00 00",       resp:"41 42 43…",status:"OK",   us:8  },
];

const ProtocolAnalyzer = () => {
  const [txns] = useState(MOCK_TRANSACTIONS);
  const [selected, setSelected] = useState(null);
  const [capturing, setCapturing] = useState(false);

  return (
    <div style={{padding:20, height:"100%", display:"flex", gap:16, overflow:"hidden"}}>
      <div style={{flex:1, display:"flex", flexDirection:"column", gap:12, overflow:"hidden"}}>
        <div style={{display:"flex", alignItems:"center", gap:8, flexShrink:0}}>
          <SectionTitle accent>Protocol Analyzer</SectionTitle>
          <div style={{flex:1}}/>
          {capturing && <Dot color="var(--red)" pulse />}
          <Btn variant={capturing?"danger":"primary"} small onClick={()=>setCapturing(!capturing)}>
            {capturing ? "⏹ Stop" : "⏺ Capture"}
          </Btn>
          <Btn variant="ghost" small>↓ Export</Btn>
        </div>

        {/* Timing diagram */}
        <Card style={{flexShrink:0}}>
          <SectionTitle accent>Signal Timing</SectionTitle>
          <div style={{display:"flex", flexDirection:"column", gap:4}}>
            {["CLK","MOSI","MISO","CS"].map((sig, si) => {
              const colors = ["var(--green)","var(--cyan)","var(--amber)","var(--red)"];
              const pattern = [
                "▁▔▁▔▁▔▁▔▁▔▁▔▁▔▁▔▁▔▁▔▁▔▁▔▁▔▁▔▁▔▁",
                "▁▁▔▔▁▁▔▔▔▁▁▁▔▔▁▁▔▔▔▁▁▔▔▁▁▔▔▁▁▔▔",
                "▁▁▁▁▔▔▁▁▁▔▔▔▁▁▁▔▁▁▁▔▔▁▁▁▔▁▁▁▁▔▔",
                "▔▔▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▔▔▔▔▔▔▔▔",
              ][si];
              return (
                <div key={sig} style={{display:"flex", alignItems:"center", gap:12}}>
                  <span style={{fontFamily:"var(--font-mono)", fontSize:10, color:colors[si], width:36}}>{sig}</span>
                  <span style={{fontFamily:"var(--font-mono)", fontSize:12, color:colors[si], letterSpacing:2, flex:1, overflow:"hidden"}}>
                    {pattern}
                  </span>
                </div>
              );
            })}
          </div>
        </Card>

        {/* Transaction table */}
        <div style={{flex:1, overflowY:"auto", borderRadius:"var(--r-lg)", border:"1px solid var(--border-dim)"}}>
          <table style={{width:"100%", borderCollapse:"collapse", fontFamily:"var(--font-mono)", fontSize:11}}>
            <thead>
              <tr style={{background:"var(--bg-base)", position:"sticky", top:0}}>
                {["#","Time","Dir","Proto","Command","TX Data","RX Data","Status","µs"].map(h=>(
                  <th key={h} style={{padding:"8px 10px", textAlign:"left", color:"var(--text-3)", fontSize:10, letterSpacing:"0.06em", textTransform:"uppercase", borderBottom:"1px solid var(--border-dim)"}}>{h}</th>
                ))}
              </tr>
            </thead>
            <tbody>
              {txns.map((t,i) => (
                <tr key={t.id} onClick={()=>setSelected(t.id===selected?null:t.id)} style={{
                  background: t.id===selected?"var(--green-glow)":(i%2===0?"var(--bg-surface)":"var(--bg-base)"),
                  borderBottom:"1px solid var(--border-dim)", cursor:"pointer"
                }}>
                  <td style={{padding:"7px 10px", color:"var(--text-3)"}}>{t.id}</td>
                  <td style={{padding:"7px 10px", color:"var(--text-3)"}}>{t.t}</td>
                  <td style={{padding:"7px 10px"}}><Badge color={t.dir==="TX"?"var(--cyan)":"var(--amber)"} small>{t.dir}</Badge></td>
                  <td style={{padding:"7px 10px"}}><Badge color={PROTOCOLS[t.proto]?.color} small>{t.proto}</Badge></td>
                  <td style={{padding:"7px 10px", color:"var(--green)", fontWeight:600}}>{t.cmd}</td>
                  <td style={{padding:"7px 10px", color:"var(--text-1)"}}>{t.data}</td>
                  <td style={{padding:"7px 10px", color:"var(--purple)"}}>{t.resp}</td>
                  <td style={{padding:"7px 10px"}}><Badge color={t.status==="OK"?"var(--green)":t.status==="BUSY"?"var(--amber)":"var(--red)"} small>{t.status}</Badge></td>
                  <td style={{padding:"7px 10px", color:"var(--text-3)"}}>{t.us}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

      {/* Detail pane */}
      {selected && (() => {
        const t = txns.find(x=>x.id===selected);
        return (
          <Card style={{width:240, flexShrink:0, height:"100%", overflowY:"auto"}}>
            <SectionTitle accent>Transaction Detail</SectionTitle>
            <div style={{display:"flex", flexDirection:"column", gap:8, fontFamily:"var(--font-mono)", fontSize:11}}>
              {Object.entries(t).map(([k,v])=>(
                <div key={k}>
                  <div style={{color:"var(--text-3)", fontSize:10, textTransform:"uppercase", letterSpacing:"0.08em"}}>{k}</div>
                  <div style={{color:"var(--text-1)", marginTop:2, wordBreak:"break-all"}}>{v}</div>
                </div>
              ))}
            </div>
          </Card>
        );
      })()}
    </div>
  );
};

export default ProtocolAnalyzer;
