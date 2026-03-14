import { useState } from 'react';
import { Btn, Card, SectionTitle } from '../components/ui';

const Settings = () => {
  const [spiSpeed, setSpiSpeed] = useState("10");
  const [i2cSpeed, setI2cSpeed] = useState("400");
  const [voltage, setVoltage] = useState("3.3");
  const [jtagSpeed, setJtagSpeed] = useState("4");

  const SliderSetting = ({label, val, set, min, max, unit, color}) => (
    <div style={{display:"flex", flexDirection:"column", gap:6}}>
      <div style={{display:"flex", justifyContent:"space-between", alignItems:"center"}}>
        <span style={{fontFamily:"var(--font-mono)", fontSize:11, color:"var(--text-2)"}}>{label}</span>
        <span style={{fontFamily:"var(--font-mono)", fontSize:13, fontWeight:700, color:color||"var(--green)"}}>{val} {unit}</span>
      </div>
      <input type="range" min={min} max={max} value={val} onChange={e=>set(e.target.value)} step="0.1"
        style={{width:"100%", accentColor:color||"var(--green)"}} />
      <div style={{display:"flex", justifyContent:"space-between", fontFamily:"var(--font-mono)", fontSize:9, color:"var(--text-3)"}}>
        <span>{min}{unit}</span><span>{max}{unit}</span>
      </div>
    </div>
  );

  const sections = [
    {
      title:"SPI Configuration",
      content:(
        <div style={{display:"flex",flexDirection:"column",gap:12}}>
          <SliderSetting label="SPI Clock Speed" val={spiSpeed} set={setSpiSpeed} min={0.1} max={40} unit="MHz" color="var(--green)" />
          <div style={{display:"grid", gridTemplateColumns:"1fr 1fr", gap:8}}>
            {[["Mode","CPOL=0 CPHA=0"],["Bit Order","MSB First"],["CS Polarity","Active Low"],["Duplex","Full"]].map(([l,v])=>(
              <div key={l} style={{background:"var(--bg-void)", padding:"8px 10px", borderRadius:"var(--r-md)", border:"1px solid var(--border-dim)"}}>
                <div style={{fontSize:10, color:"var(--text-3)", fontFamily:"var(--font-mono)"}}>{l}</div>
                <div style={{fontSize:11, color:"var(--green)", fontFamily:"var(--font-mono)", marginTop:2}}>{v}</div>
              </div>
            ))}
          </div>
        </div>
      )
    },
    {
      title:"I²C Configuration",
      content: <SliderSetting label="I2C Speed" val={i2cSpeed} set={setI2cSpeed} min={10} max={1000} unit="KHz" color="var(--cyan)" />
    },
    {
      title:"JTAG/SWD Configuration",
      content: <SliderSetting label="JTAG Clock" val={jtagSpeed} set={setJtagSpeed} min={0.1} max={20} unit="MHz" color="var(--amber)" />
    },
    {
      title:"Power & Voltage",
      content:(
        <div>
          <SliderSetting label="Target Voltage" val={voltage} set={setVoltage} min={1.8} max={5.0} unit="V" color="var(--red)" />
          <div style={{marginTop:8, padding:"8px 10px", background:"var(--amber)11", border:"1px solid var(--amber)33", borderRadius:"var(--r-md)"}}>
            <span style={{fontFamily:"var(--font-mono)", fontSize:10, color:"var(--amber)"}}>⚠ Check chip datasheet before changing voltage!</span>
          </div>
        </div>
      )
    }
  ];

  return (
    <div style={{padding:20, height:"100%", overflowY:"auto"}}>
      <SectionTitle accent>Settings & Configuration</SectionTitle>
      <div style={{display:"grid", gridTemplateColumns:"1fr 1fr", gap:12, marginTop:8}}>
        {sections.map(s=>(
          <Card key={s.title}>
            <div style={{fontFamily:"var(--font-display)", fontSize:12, fontWeight:700, color:"var(--text-2)", letterSpacing:"0.08em", textTransform:"uppercase", marginBottom:14}}>{s.title}</div>
            {s.content}
          </Card>
        ))}
        <Card>
          <div style={{fontFamily:"var(--font-display)", fontSize:12, fontWeight:700, color:"var(--text-2)", letterSpacing:"0.08em", textTransform:"uppercase", marginBottom:14}}>RP2040 Bridge Firmware</div>
          <div style={{display:"flex", flexDirection:"column", gap:8}}>
            {[["Version","2.4.1"],["Build","2025-03-10"],["USB PID","0x2E8A"],["CDC ACM","12 Mbps"],["Protocol Slots","32"]].map(([k,v])=>(
              <div key={k} style={{display:"flex",justifyContent:"space-between",fontFamily:"var(--font-mono)",fontSize:11,borderBottom:"1px solid var(--border-dim)",paddingBottom:4}}>
                <span style={{color:"var(--text-3)"}}>{k}</span>
                <span style={{color:"var(--green)"}}>{v}</span>
              </div>
            ))}
            <Btn variant="amber" small icon="↑" style={{marginTop:4}}>Check for Firmware Update</Btn>
          </div>
        </Card>
        <Card>
          <div style={{fontFamily:"var(--font-display)", fontSize:12, fontWeight:700, color:"var(--text-2)", letterSpacing:"0.08em", textTransform:"uppercase", marginBottom:14}}>Server Connection</div>
          <div style={{display:"flex", flexDirection:"column", gap:8}}>
            {[["Endpoint","wss://api.uniprog.io"],["Status","Connected"],["Latency","2ms"],["DB Version","4847 chips"],["TLS","TLS 1.3"]].map(([k,v])=>(
              <div key={k} style={{display:"flex",justifyContent:"space-between",fontFamily:"var(--font-mono)",fontSize:11,borderBottom:"1px solid var(--border-dim)",paddingBottom:4}}>
                <span style={{color:"var(--text-3)"}}>{k}</span>
                <span style={{color:"var(--cyan)"}}>{v}</span>
              </div>
            ))}
          </div>
        </Card>
      </div>
    </div>
  );
};

export default Settings;
