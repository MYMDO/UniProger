import { useState, useEffect, useRef, useCallback } from "react";

// ── Fonts & Global Styles ────────────────────────────────────────────────────
const GlobalStyles = () => (
  <style>{`
    @import url('https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@300;400;500;600;700&family=Syne:wght@400;500;600;700;800&family=DM+Sans:wght@300;400;500;600&display=swap');

    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    :root {
      --bg-void:     #060809;
      --bg-base:     #0c1014;
      --bg-surface:  #111820;
      --bg-raised:   #162028;
      --bg-overlay:  #1c2a36;
      --bg-hover:    #1f3040;

      --green:       #00ffa3;
      --green-dim:   #00cc80;
      --green-glow:  rgba(0,255,163,0.15);
      --cyan:        #00d4ff;
      --cyan-dim:    #00aacc;
      --amber:       #ffb800;
      --amber-dim:   #cc9200;
      --red:         #ff4466;
      --red-dim:     #cc2244;
      --purple:      #b06aff;

      --border:      rgba(0,255,163,0.12);
      --border-dim:  rgba(255,255,255,0.06);
      --border-hover:rgba(0,255,163,0.3);

      --text-1:      #e8f4f0;
      --text-2:      #8ba89e;
      --text-3:      #4a6560;
      --text-accent: #00ffa3;

      --font-mono:   'JetBrains Mono', monospace;
      --font-display:'Syne', sans-serif;
      --font-ui:     'DM Sans', sans-serif;

      --r-sm: 4px;
      --r-md: 8px;
      --r-lg: 12px;
      --r-xl: 16px;
    }

    html, body, #root { height: 100%; background: var(--bg-void); color: var(--text-1); font-family: var(--font-ui); overflow: hidden; }

    ::-webkit-scrollbar { width: 4px; height: 4px; }
    ::-webkit-scrollbar-track { background: var(--bg-base); }
    ::-webkit-scrollbar-thumb { background: var(--bg-overlay); border-radius: 2px; }
    ::-webkit-scrollbar-thumb:hover { background: var(--green-dim); }

    .scanline {
      position: fixed; top:0; left:0; right:0; bottom:0;
      background: repeating-linear-gradient(0deg, transparent, transparent 2px, rgba(0,0,0,0.03) 2px, rgba(0,0,0,0.03) 4px);
      pointer-events: none; z-index: 9999;
    }

    @keyframes pulse-green { 0%,100%{opacity:1;box-shadow:0 0 6px var(--green)} 50%{opacity:0.6;box-shadow:0 0 2px var(--green)} }
    @keyframes pulse-amber { 0%,100%{opacity:1} 50%{opacity:0.4} }
    @keyframes sweep { from{width:0} to{width:100%} }
    @keyframes fadeIn { from{opacity:0;transform:translateY(4px)} to{opacity:1;transform:translateY(0)} }
    @keyframes slideIn { from{opacity:0;transform:translateX(-8px)} to{opacity:1;transform:translateX(0)} }
    @keyframes spin { to{transform:rotate(360deg)} }
    @keyframes blink { 0%,100%{opacity:1} 50%{opacity:0} }
    @keyframes dataflow { 0%{background-position:0 0} 100%{background-position:40px 0} }
    @keyframes glitch {
      0%,100%{clip-path:inset(0 0 98% 0)} 20%{clip-path:inset(30% 0 50% 0)}
      40%{clip-path:inset(70% 0 10% 0)} 60%{clip-path:inset(10% 0 70% 0)}
      80%{clip-path:inset(50% 0 30% 0)}
    }

    .animate-in { animation: fadeIn 0.25s ease forwards; }
    .slide-in   { animation: slideIn 0.2s ease forwards; }

    button { cursor: pointer; border: none; outline: none; font-family: var(--font-ui); }
    input, select, textarea { outline: none; font-family: var(--font-mono); }

    .tooltip-wrap { position: relative; }
    .tooltip-wrap:hover .tooltip { opacity: 1; pointer-events: none; }
    .tooltip {
      position: absolute; bottom: calc(100% + 6px); left: 50%; transform: translateX(-50%);
      background: var(--bg-overlay); border: 1px solid var(--border);
      color: var(--text-1); font-size: 10px; font-family: var(--font-mono);
      padding: 4px 8px; border-radius: var(--r-sm); white-space: nowrap;
      opacity: 0; transition: opacity 0.15s; z-index: 100;
    }
  `}</style>
);

// ── Constants & Mock Data ─────────────────────────────────────────────────────
const PROTOCOLS = {
  SPI:      { color: "#00ffa3", icon: "◈", pins: ["MOSI","MISO","CLK","CS"] },
  I2C:      { color: "#00d4ff", icon: "◉", pins: ["SDA","SCL"] },
  JTAG:     { color: "#ffb800", icon: "◆", pins: ["TDI","TDO","TCK","TMS","TRST"] },
  SWD:      { color: "#b06aff", icon: "◇", pins: ["SWDIO","SWCLK"] },
  UART:     { color: "#ff6644", icon: "◈", pins: ["TX","RX"] },
  "1-Wire": { color: "#ff4466", icon: "○", pins: ["DQ"] },
  PARALLEL: { color: "#88ff00", icon: "▣", pins: ["D0-D7","ALE","WE","RE","CE"] },
  SWI:      { color: "#00ffdd", icon: "◐", pins: ["SWI"] },
};

const CHIP_DB = [
  { id:1,  name:"W25Q128JV",   mfr:"Winbond",   type:"SPI Flash",   size:"128Mb",  proto:"SPI",  voltage:"3.3V", pkg:"SOP8",   ops:["read","write","erase","verify","otp"], time:"~45s" },
  { id:2,  name:"W25Q64FV",    mfr:"Winbond",   type:"SPI Flash",   size:"64Mb",   proto:"SPI",  voltage:"3.3V", pkg:"SOP8",   ops:["read","write","erase","verify","otp"], time:"~22s" },
  { id:3,  name:"MX25L12835F", mfr:"Macronix",  type:"SPI Flash",   size:"128Mb",  proto:"SPI",  voltage:"3.3V", pkg:"SOP16",  ops:["read","write","erase","verify"],       time:"~40s" },
  { id:4,  name:"AT24C256",    mfr:"Microchip", type:"I2C EEPROM",  size:"256Kb",  proto:"I2C",  voltage:"5V",   pkg:"DIP8",   ops:["read","write","verify"],               time:"~3s"  },
  { id:5,  name:"AT25DF321A",  mfr:"Adesto",    type:"SPI Flash",   size:"32Mb",   proto:"SPI",  voltage:"3.3V", pkg:"SOP8",   ops:["read","write","erase","verify"],       time:"~18s" },
  { id:6,  name:"STM32F103C8", mfr:"ST Micro",  type:"MCU Flash",   size:"64KB",   proto:"SWD",  voltage:"3.3V", pkg:"LQFP48", ops:["read","write","erase","verify","debug"],time:"~8s"  },
  { id:7,  name:"ATmega328P",  mfr:"Microchip", type:"MCU Flash",   size:"32KB",   proto:"SPI",  voltage:"5V",   pkg:"DIP28",  ops:["read","write","erase","verify","fuse"], time:"~6s"  },
  { id:8,  name:"GD25Q64C",    mfr:"GigaDevice",type:"SPI Flash",   size:"64Mb",   proto:"SPI",  voltage:"3.3V", pkg:"SOP8",   ops:["read","write","erase","verify"],       time:"~25s" },
  { id:9,  name:"nRF52840",    mfr:"Nordic",    type:"MCU Flash",   size:"1MB",    proto:"SWD",  voltage:"3V",   pkg:"QFN73",  ops:["read","write","erase","verify","debug"],time:"~12s" },
  { id:10, name:"ESP32-S3",    mfr:"Espressif", type:"MCU Flash",   size:"8MB",    proto:"JTAG", voltage:"3.3V", pkg:"QFN56",  ops:["read","write","erase","verify"],       time:"~30s" },
  { id:11, name:"24LC512",     mfr:"Microchip", type:"I2C EEPROM",  size:"512Kb",  proto:"I2C",  voltage:"5V",   pkg:"DIP8",   ops:["read","write","verify"],               time:"~5s"  },
  { id:12, name:"SST25VF016B", mfr:"Microchip", type:"SPI Flash",   size:"16Mb",   proto:"SPI",  voltage:"3.3V", pkg:"SOP16",  ops:["read","write","erase","verify"],       time:"~12s" },
  { id:13, name:"DS18B20",     mfr:"Maxim",     type:"1-Wire ROM",  size:"256B",   proto:"1-Wire",voltage:"3.3V",pkg:"TO-92",  ops:["read","write"],                        time:"~0.5s"},
  { id:14, name:"M95256",      mfr:"ST Micro",  type:"SPI EEPROM",  size:"256Kb",  proto:"SPI",  voltage:"3.3V", pkg:"SOP8",   ops:["read","write","verify"],               time:"~2s"  },
  { id:15, name:"RP2040",      mfr:"Raspberry", type:"MCU Flash",   size:"2MB",    proto:"SWD",  voltage:"3.3V", pkg:"QFN56",  ops:["read","write","erase","verify"],       time:"~10s" },
];

const LOG_COLORS = { INFO:"var(--cyan)", WARN:"var(--amber)", ERR:"var(--red)", OK:"var(--green)", TX:"var(--purple)", RX:"var(--text-2)" };

function genHexRows(count = 16, seed = 0xA0) {
  const rows = [];
  for (let r = 0; r < count; r++) {
    const addr = (r * 16).toString(16).padStart(8,"0").toUpperCase();
    const bytes = Array.from({length:16}, (_, i) => ((seed + r * 16 + i) & 0xFF));
    const hex = bytes.map(b => b.toString(16).padStart(2,"0").toUpperCase());
    const ascii = bytes.map(b => (b >= 32 && b < 127) ? String.fromCharCode(b) : "·");
    rows.push({ addr, hex, ascii });
  }
  return rows;
}

const INIT_LOGS = [
  { t:"00:00.001", level:"INFO", msg:"RP2040 Universal Bridge v2.4.1 initialized" },
  { t:"00:00.003", level:"OK",   msg:"USB CDC connected @ 12Mbps" },
  { t:"00:00.005", level:"INFO", msg:"Protocol engine ready — SPI/I2C/JTAG/SWD/1-Wire/PARALLEL" },
  { t:"00:00.006", level:"INFO", msg:"Chip database loaded: 4,847 entries" },
  { t:"00:00.008", level:"OK",   msg:"Server handshake complete — latency 2ms" },
];

// ── Small Components ──────────────────────────────────────────────────────────
const Badge = ({ color, children, small }) => (
  <span style={{
    display:"inline-flex", alignItems:"center", gap:4,
    padding: small ? "1px 6px" : "2px 8px",
    fontSize: small ? 9 : 10, fontFamily:"var(--font-mono)", fontWeight:600,
    letterSpacing:"0.08em", textTransform:"uppercase",
    color: color || "var(--green)", background: `${color||"var(--green)"}18`,
    border: `1px solid ${color||"var(--green)"}44`,
    borderRadius: "var(--r-sm)"
  }}>{children}</span>
);

const Dot = ({ color, pulse }) => (
  <span style={{
    width:7, height:7, borderRadius:"50%",
    background: color || "var(--green)",
    display:"inline-block", flexShrink:0,
    animation: pulse ? "pulse-green 1.8s ease-in-out infinite" : "none",
    boxShadow: `0 0 6px ${color || "var(--green)"}`
  }} />
);

const Btn = ({ children, variant="primary", onClick, disabled, small, icon, style={} }) => {
  const base = {
    display:"inline-flex", alignItems:"center", gap:6,
    padding: small ? "5px 12px" : "8px 18px",
    fontSize: small ? 11 : 12, fontFamily:"var(--font-ui)", fontWeight:600,
    letterSpacing:"0.04em", borderRadius:"var(--r-md)",
    transition:"all 0.15s", cursor: disabled ? "not-allowed" : "pointer",
    opacity: disabled ? 0.4 : 1, ...style
  };
  const variants = {
    primary:  { background:"var(--green)", color:"var(--bg-void)", border:"none", boxShadow:"0 0 12px var(--green-glow)" },
    ghost:    { background:"transparent", color:"var(--text-1)", border:"1px solid var(--border-dim)" },
    danger:   { background:"var(--red)22", color:"var(--red)", border:"1px solid var(--red)44" },
    amber:    { background:"var(--amber)22", color:"var(--amber)", border:"1px solid var(--amber)44" },
    cyan:     { background:"var(--cyan)22", color:"var(--cyan)", border:"1px solid var(--cyan)44" },
  };
  return (
    <button onClick={disabled ? undefined : onClick} style={{...base, ...variants[variant]}}>
      {icon && <span style={{fontSize:14}}>{icon}</span>}
      {children}
    </button>
  );
};

const Card = ({ children, style={}, glow }) => (
  <div style={{
    background:"var(--bg-surface)", border:`1px solid ${glow?"var(--border)":"var(--border-dim)"}`,
    borderRadius:"var(--r-lg)", padding:"16px",
    boxShadow: glow ? "0 0 24px var(--green-glow)" : "none",
    ...style
  }}>{children}</div>
);

const ProgressBar = ({ value, color="var(--green)", animated }) => (
  <div style={{background:"var(--bg-void)", borderRadius:3, height:4, overflow:"hidden", position:"relative"}}>
    <div style={{
      height:"100%", width:`${value}%`, background: color,
      borderRadius:3, transition:"width 0.1s linear",
      boxShadow:`0 0 8px ${color}`,
      backgroundImage: animated ? `repeating-linear-gradient(90deg,transparent,transparent 8px,${color}44 8px,${color}44 16px)` : "none",
      backgroundSize:"40px 100%",
      animation: animated && value > 0 && value < 100 ? "dataflow 0.8s linear infinite" : "none"
    }} />
  </div>
);

const SectionTitle = ({ children, accent }) => (
  <div style={{display:"flex", alignItems:"center", gap:8, marginBottom:12}}>
    {accent && <div style={{width:3,height:14,background:"var(--green)",borderRadius:2,boxShadow:"0 0 6px var(--green)"}} />}
    <span style={{fontFamily:"var(--font-display)", fontSize:11, fontWeight:700, letterSpacing:"0.12em", textTransform:"uppercase", color:"var(--text-2)"}}>{children}</span>
  </div>
);

// ── Connection Status Bar ─────────────────────────────────────────────────────
const StatusBar = ({ connected, deviceInfo, serverOk, onConnect, onDisconnect }) => (
  <div style={{
    display:"flex", alignItems:"center", gap:16, padding:"0 20px",
    height:40, borderBottom:"1px solid var(--border-dim)",
    background:"var(--bg-base)", flexShrink:0,
  }}>
    <div style={{display:"flex", alignItems:"center", gap:6}}>
      <Dot color={connected?"var(--green)":"var(--red)"} pulse={connected} />
      <span style={{fontFamily:"var(--font-mono)", fontSize:11, color:"var(--text-2)"}}>
        {connected ? deviceInfo.port : "NOT CONNECTED"}
      </span>
    </div>
    {connected && <>
      <div style={{width:1, height:16, background:"var(--border-dim)"}} />
      <span style={{fontFamily:"var(--font-mono)", fontSize:10, color:"var(--text-3)"}}>
        RP2040 Bridge v{deviceInfo.fw} · {deviceInfo.speed}
      </span>
      <div style={{width:1, height:16, background:"var(--border-dim)"}} />
      <Badge color="var(--cyan)" small>USB CDC</Badge>
      <Badge color={serverOk?"var(--green)":"var(--amber)"} small>{serverOk?"SERVER OK":"OFFLINE"}</Badge>
    </>}
    <div style={{marginLeft:"auto", display:"flex", gap:8, alignItems:"center"}}>
      <span style={{fontFamily:"var(--font-mono)", fontSize:10, color:"var(--text-3)"}}>
        {new Date().toLocaleTimeString()}
      </span>
      {connected
        ? <Btn variant="danger" small onClick={onDisconnect}>⏏ Disconnect</Btn>
        : <Btn variant="primary" small onClick={onConnect} icon="⚡">Connect Device</Btn>
      }
    </div>
  </div>
);

// ── Sidebar Navigation ────────────────────────────────────────────────────────
const NAV_ITEMS = [
  { id:"dashboard",  label:"Dashboard",    icon:"◈" },
  { id:"chips",      label:"Chip Database",icon:"⊞" },
  { id:"operation",  label:"Operations",   icon:"▶" },
  { id:"hexeditor",  label:"Hex Editor",   icon:"≡" },
  { id:"analyzer",   label:"Protocol Analyzer",icon:"〜" },
  { id:"settings",   label:"Settings",     icon:"◎" },
  { id:"firmware",   label:"Firmware",     icon:"↑" },
];

const Sidebar = ({ active, onChange, connected }) => (
  <div style={{
    width:56, flexShrink:0, background:"var(--bg-base)",
    borderRight:"1px solid var(--border-dim)", display:"flex", flexDirection:"column",
    alignItems:"center", padding:"12px 0", gap:2,
  }}>
    <div style={{
      width:32, height:32, borderRadius:"var(--r-md)", marginBottom:16,
      background:"var(--green)", display:"flex", alignItems:"center", justifyContent:"center",
      boxShadow:"0 0 16px var(--green-glow)"
    }}>
      <span style={{fontSize:16, color:"var(--bg-void)", fontWeight:900}}>⊕</span>
    </div>
    {NAV_ITEMS.map(item => (
      <div key={item.id} className="tooltip-wrap" onClick={() => onChange(item.id)} style={{
        width:40, height:40, borderRadius:"var(--r-md)", cursor:"pointer",
        display:"flex", alignItems:"center", justifyContent:"center", fontSize:18,
        transition:"all 0.15s",
        background: active===item.id ? "var(--green-glow)" : "transparent",
        color: active===item.id ? "var(--green)" : (connected ? "var(--text-3)" : "var(--text-3)"),
        border: active===item.id ? "1px solid var(--border)" : "1px solid transparent",
        filter: !connected && item.id !== "dashboard" ? "opacity(0.4)" : "none",
      }}>
        {item.icon}
        <span className="tooltip">{item.label}</span>
      </div>
    ))}
  </div>
);

// ── Dashboard View ────────────────────────────────────────────────────────────
const Dashboard = ({ connected, onConnect, chipSelected, onSelectChip }) => {
  const [autoDetecting, setAutoDetecting] = useState(false);
  const [detected, setDetected] = useState(null);

  const runAutoDetect = () => {
    setAutoDetecting(true);
    setDetected(null);
    setTimeout(() => {
      setDetected(CHIP_DB[0]);
      setAutoDetecting(false);
    }, 2200);
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
                    <Badge color={PROTOCOLS[detected.proto].color}>{detected.proto}</Badge>
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

// ── Chip Database View ────────────────────────────────────────────────────────
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

  const inputStyle = {
    background:"var(--bg-void)", border:"1px solid var(--border-dim)", borderRadius:"var(--r-md)",
    color:"var(--text-1)", fontFamily:"var(--font-mono)", fontSize:12, padding:"8px 12px",
    width:"100%", transition:"border-color 0.15s"
  };

  return (
    <div style={{padding:20, height:"100%", display:"flex", flexDirection:"column", gap:16}}>
      <SectionTitle accent>Chip Database — {CHIP_DB.length} Entries</SectionTitle>

      {/* Filters */}
      <div style={{display:"flex", gap:8, alignItems:"center"}}>
        <input
          style={{...inputStyle, flex:1}}
          placeholder="Search by name, manufacturer, type…"
          value={search}
          onChange={e=>setSearch(e.target.value)}
        />
        <select value={filterProto} onChange={e=>setFilterProto(e.target.value)}
          style={{...inputStyle, width:"auto", cursor:"pointer"}}>
          {protos.map(p => <option key={p} value={p}>{p}</option>)}
        </select>
        <select value={filterType} onChange={e=>setFilterType(e.target.value)}
          style={{...inputStyle, width:"auto", cursor:"pointer"}}>
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
                  outline: isSelected ? `1px solid var(--border)` : "none"
                }}
                onMouseEnter={e=>!isSelected&&(e.currentTarget.style.background="var(--bg-hover)")}
                onMouseLeave={e=>!isSelected&&(e.currentTarget.style.background=i%2===0?"var(--bg-surface)":"var(--bg-base)")}
                >
                  <td style={{padding:"9px 12px", color:"var(--green)", fontWeight:600}}>{chip.name}</td>
                  <td style={{padding:"9px 12px", color:"var(--text-2)"}}>{chip.mfr}</td>
                  <td style={{padding:"9px 12px", color:"var(--text-2)"}}>{chip.type}</td>
                  <td style={{padding:"9px 12px", color:"var(--text-1)"}}>{chip.size}</td>
                  <td style={{padding:"9px 12px"}}>
                    <Badge color={proto?.color} small>{chip.proto}</Badge>
                  </td>
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

// ── Operations View ───────────────────────────────────────────────────────────
const Operations = ({ chip, connected }) => {
  const [operation, setOperation] = useState(null); // null | 'read' | 'write' | 'erase' | 'verify'
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
    const startLogs = [
      { t: "now", level:"INFO", msg:`Starting ${op.toUpperCase()} · addr:0x${address} len:0x${length}` },
      { t: "now", level:"TX",   msg:`CMD: 0x${op==='erase'?'C7':'03'} → RP2040 bridge` },
    ];
    setLogs(startLogs);
    let p = 0;
    timerRef.current = setInterval(() => {
      p += Math.random() * 3 + 1;
      if (p >= 100) { p = 100; clearInterval(timerRef.current); setDone(true);
        setLogs(l => [...l, { t:"now", level:"OK", msg:`${op.toUpperCase()} completed successfully · CRC32: 0xA3F4C21B` }]);
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

  const OPS = [
    { id:"read",   label:"Read",   icon:"↓", color:"var(--cyan)",  desc:"Read chip contents to buffer" },
    { id:"write",  label:"Write",  icon:"↑", color:"var(--green)", desc:"Write buffer to chip" },
    { id:"erase",  label:"Erase",  icon:"⊠", color:"var(--red)",   desc:"Full chip erase" },
    { id:"verify", label:"Verify", icon:"✓", color:"var(--amber)", desc:"Compare chip vs buffer" },
  ];

  const inputStyle = {
    background:"var(--bg-void)", border:"1px solid var(--border-dim)",
    borderRadius:"var(--r-sm)", color:"var(--text-1)",
    fontFamily:"var(--font-mono)", fontSize:12, padding:"6px 10px", width:"100%"
  };

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
          <div style={{marginTop:8}}>
            <Badge color={PROTOCOLS[chip.proto]?.color}>{chip.proto}</Badge>
          </div>
        </Card>

        <Card>
          <SectionTitle accent>Address Range</SectionTitle>
          <div style={{display:"flex", flexDirection:"column", gap:8}}>
            {[["Start Address","address",address,setAddress],["Length","length",length,setLength]].map(([l,k,v,set])=>(
              <div key={k}>
                <div style={{fontSize:10, color:"var(--text-3)", marginBottom:3, fontFamily:"var(--font-mono)", letterSpacing:"0.08em"}}>{l}</div>
                <div style={{display:"flex", alignItems:"center"}}>
                  <span style={{fontFamily:"var(--font-mono)", fontSize:12, color:"var(--text-3)", marginRight:4}}>0x</span>
                  <input style={inputStyle} value={v} onChange={e=>set(e.target.value.replace(/[^0-9A-Fa-f]/g,"").slice(0,8))} disabled={opActive} />
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
        {/* Progress */}
        <Card style={{flexShrink:0}}>
          <SectionTitle accent>Progress</SectionTitle>
          {operation ? (
            <div style={{display:"flex", flexDirection:"column", gap:10}}>
              <div style={{display:"flex", justifyContent:"space-between", alignItems:"center"}}>
                <span style={{fontFamily:"var(--font-mono)", fontSize:12, color:"var(--text-2)"}}>
                  {operation.toUpperCase()} · {chip.name}
                </span>
                <span style={{fontFamily:"var(--font-mono)", fontSize:14, fontWeight:700, color: done?"var(--green)":"var(--text-1)"}}>
                  {progress.toFixed(1)}%
                </span>
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
            <div style={{color:"var(--text-3)", fontFamily:"var(--font-mono)", fontSize:12}}>
              Select an operation to begin.
            </div>
          )}
        </Card>

        {/* Log */}
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

// ── Hex Editor ────────────────────────────────────────────────────────────────
const HexEditor = ({ chip }) => {
  const [rows] = useState(() => genHexRows(32, 0x41));
  const [selected, setSelected] = useState(null);
  const [editMode, setEditMode] = useState(false);
  const [search, setSearch] = useState("");
  const [baseAddr, setBaseAddr] = useState(0);

  return (
    <div style={{padding:20, height:"100%", display:"flex", flexDirection:"column", gap:12, overflow:"hidden"}}>
      {/* Toolbar */}
      <div style={{display:"flex", gap:8, alignItems:"center", flexShrink:0}}>
        <SectionTitle accent>Hex Editor</SectionTitle>
        <div style={{flex:1}} />
        <input
          placeholder="Find hex bytes (e.g. FF A0 00)"
          value={search} onChange={e=>setSearch(e.target.value)}
          style={{
            background:"var(--bg-surface)", border:"1px solid var(--border-dim)",
            borderRadius:"var(--r-md)", color:"var(--text-1)",
            fontFamily:"var(--font-mono)", fontSize:11, padding:"6px 12px", width:240
          }}
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
                const isFF   = b === "FF";
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
          Size: {(rows.length*16).toString(16).toUpperCase()} bytes · Encoding: HEX · {editMode ? "EDIT MODE" : "READ-ONLY"}
        </span>
      </div>
    </div>
  );
};

// ── Protocol Analyzer ─────────────────────────────────────────────────────────
const MOCK_TRANSACTIONS = [
  { id:1, t:"00:00.001", dir:"TX", proto:"SPI", cmd:"CMD_JEDEC_ID",   data:"9F",               resp:"EF 40 18", status:"OK",  us:2  },
  { id:2, t:"00:00.003", dir:"TX", proto:"SPI", cmd:"CMD_READ_STATUS",data:"05",               resp:"00",       status:"OK",  us:1  },
  { id:3, t:"00:00.005", dir:"TX", proto:"SPI", cmd:"CMD_WRITE_EN",   data:"06",               resp:"—",        status:"OK",  us:1  },
  { id:4, t:"00:00.008", dir:"TX", proto:"SPI", cmd:"CMD_SECTOR_ERASE",data:"20 00 00 00",     resp:"—",        status:"OK",  us:45 },
  { id:5, t:"00:00.053", dir:"RX", proto:"SPI", cmd:"STATUS_POLL",    data:"05",               resp:"01",       status:"BUSY",us:1  },
  { id:6, t:"00:00.100", dir:"RX", proto:"SPI", cmd:"STATUS_POLL",    data:"05",               resp:"00",       status:"OK",  us:1  },
  { id:7, t:"00:00.101", dir:"TX", proto:"SPI", cmd:"CMD_PAGE_PROG",  data:"02 00 00 00 + 256B",resp:"—",       status:"OK",  us:3  },
  { id:8, t:"00:00.104", dir:"TX", proto:"SPI", cmd:"CMD_READ_DATA",  data:"03 00 00 00",      resp:"41 42 43…",status:"OK",  us:8  },
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

        {/* Timing diagram mock */}
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
                  <td style={{padding:"7px 10px"}}>
                    <Badge color={t.dir==="TX"?"var(--cyan)":"var(--amber)"} small>{t.dir}</Badge>
                  </td>
                  <td style={{padding:"7px 10px"}}>
                    <Badge color={PROTOCOLS[t.proto]?.color} small>{t.proto}</Badge>
                  </td>
                  <td style={{padding:"7px 10px", color:"var(--green)", fontWeight:600}}>{t.cmd}</td>
                  <td style={{padding:"7px 10px", color:"var(--text-1)"}}>{t.data}</td>
                  <td style={{padding:"7px 10px", color:"var(--purple)"}}>{t.resp}</td>
                  <td style={{padding:"7px 10px"}}>
                    <Badge color={t.status==="OK"?"var(--green)":t.status==="BUSY"?"var(--amber)":"var(--red)"} small>{t.status}</Badge>
                  </td>
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

// ── Settings View ─────────────────────────────────────────────────────────────
const Settings = () => {
  const [spiSpeed, setSpiSpeed] = useState("10");
  const [i2cSpeed, setI2cSpeed] = useState("400");
  const [voltage, setVoltage] = useState("3.3");
  const [jtagSpeed, setJtagSpeed] = useState("4");

  const SliderSetting = ({label, val, set, min, max, unit, color}) => (
    <div style={{display:"flex", flexDirection:"column", gap:6}}>
      <div style={{display:"flex", justifyContent:"space-between", alignItems:"center"}}>
        <span style={{fontFamily:"var(--font-mono)", fontSize:11, color:"var(--text-2)"}}>{label}</span>
        <span style={{fontFamily:"var(--font-mono)", fontSize:13, fontWeight:700, color:color||"var(--green)"}}>
          {val} {unit}
        </span>
      </div>
      <input type="range" min={min} max={max} value={val} onChange={e=>set(e.target.value)}
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
      content:(
        <SliderSetting label="I2C Speed" val={i2cSpeed} set={setI2cSpeed} min={10} max={1000} unit="KHz" color="var(--cyan)" />
      )
    },
    {
      title:"JTAG/SWD Configuration",
      content:(
        <SliderSetting label="JTAG Clock" val={jtagSpeed} set={setJtagSpeed} min={0.1} max={20} unit="MHz" color="var(--amber)" />
      )
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

// ── Firmware Update View ──────────────────────────────────────────────────────
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

// ── Connect Modal ─────────────────────────────────────────────────────────────
const ConnectModal = ({ onClose, onConnect }) => {
  const [step, setStep] = useState(0);

  useEffect(() => {
    if (step === 0) { setTimeout(() => setStep(1), 800); }
    else if (step === 1) { setTimeout(() => setStep(2), 1200); }
    else if (step === 2) { setTimeout(() => setStep(3), 900); }
    else if (step === 3) { setTimeout(() => { onConnect(); onClose(); }, 600); }
  }, [step]);

  const steps = [
    { label:"Requesting serial port…", done: step>0 },
    { label:"Opening USB CDC @ 115200 baud…", done: step>1 },
    { label:"Handshaking with RP2040 bridge…", done: step>2 },
    { label:"Server authentication…", done: step>3 },
  ];

  return (
    <div style={{
      position:"fixed", inset:0, background:"rgba(6,8,9,0.85)", backdropFilter:"blur(8px)",
      display:"flex", alignItems:"center", justifyContent:"center", zIndex:1000
    }}>
      <div style={{
        background:"var(--bg-surface)", border:"1px solid var(--border)",
        borderRadius:"var(--r-xl)", padding:32, width:380,
        boxShadow:"0 0 60px var(--green-glow)"
      }} className="animate-in">
        <div style={{fontFamily:"var(--font-display)", fontSize:16, fontWeight:800, color:"var(--green)", marginBottom:6}}>
          Connecting Device
        </div>
        <div style={{fontSize:12, color:"var(--text-3)", marginBottom:24}}>
          Web Serial API · Chrome/Edge required
        </div>
        <div style={{display:"flex", flexDirection:"column", gap:10}}>
          {steps.map((s, i) => (
            <div key={i} style={{display:"flex", alignItems:"center", gap:10, fontFamily:"var(--font-mono)", fontSize:12}}>
              {s.done
                ? <span style={{color:"var(--green)", fontSize:14}}>✓</span>
                : i === step
                  ? <span style={{animation:"spin 0.8s linear infinite", display:"inline-block", color:"var(--amber)"}}>⟳</span>
                  : <span style={{color:"var(--text-3)"}}>○</span>
              }
              <span style={{color: s.done ? "var(--text-1)" : i === step ? "var(--amber)" : "var(--text-3)"}}>{s.label}</span>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};

// ── App Shell ─────────────────────────────────────────────────────────────────
export default function App() {
  const [view, setView] = useState("dashboard");
  const [connected, setConnected] = useState(false);
  const [showModal, setShowModal] = useState(false);
  const [selectedChip, setSelectedChip] = useState(null);
  const [clock, setClock] = useState(new Date().toLocaleTimeString());
  const [serverOk] = useState(true);

  const deviceInfo = { port:"COM4 · USB (RP2040)", fw:"2.4.1", speed:"12 Mbps" };

  useEffect(() => {
    const t = setInterval(() => setClock(new Date().toLocaleTimeString()), 1000);
    return () => clearInterval(t);
  }, []);

  const handleSelectChip = (chip) => {
    setSelectedChip(chip);
    setView("operation");
  };

  return (
    <>
      <GlobalStyles />
      <div className="scanline" />
      {showModal && <ConnectModal onClose={()=>setShowModal(false)} onConnect={()=>setConnected(true)} />}
      <div style={{display:"flex", flexDirection:"column", height:"100vh", overflow:"hidden"}}>
        {/* Top bar */}
        <div style={{
          display:"flex", alignItems:"center", gap:0,
          height:48, background:"var(--bg-base)", borderBottom:"1px solid var(--border-dim)",
          flexShrink:0, paddingLeft:56+1, // align with sidebar
        }}>
          <div style={{
            display:"flex", alignItems:"center", gap:10, padding:"0 20px",
            borderRight:"1px solid var(--border-dim)", height:"100%"
          }}>
            <span style={{fontFamily:"var(--font-display)", fontSize:14, fontWeight:800, color:"var(--green)", letterSpacing:"0.04em"}}>
              UNIPROG
            </span>
            <Badge small>SaaS v2.4</Badge>
          </div>
          <StatusBar
            connected={connected} deviceInfo={deviceInfo} serverOk={serverOk}
            onConnect={()=>setShowModal(true)} onDisconnect={()=>setConnected(false)}
          />
        </div>

        <div style={{display:"flex", flex:1, overflow:"hidden"}}>
          <Sidebar active={view} onChange={v=>connected?setView(v):v==="dashboard"&&setView(v)} connected={connected} />
          <div style={{flex:1, overflow:"hidden", animation:"fadeIn 0.2s ease"}}>
            {view==="dashboard"   && <Dashboard connected={connected} onConnect={()=>setShowModal(true)} chipSelected={selectedChip} onSelectChip={handleSelectChip} />}
            {view==="chips"       && <ChipDatabase onSelectChip={handleSelectChip} selectedChip={selectedChip} />}
            {view==="operation"   && <Operations chip={selectedChip} connected={connected} />}
            {view==="hexeditor"   && <HexEditor chip={selectedChip} />}
            {view==="analyzer"    && <ProtocolAnalyzer />}
            {view==="settings"    && <Settings />}
            {view==="firmware"    && <FirmwareUpdate connected={connected} />}
          </div>
        </div>
      </div>
    </>
  );
}
