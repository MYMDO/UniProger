const NAV_ITEMS = [
  { id:"dashboard",  label:"Dashboard",         icon:"◈" },
  { id:"chips",      label:"Chip Database",      icon:"⊞" },
  { id:"operation",  label:"Operations",         icon:"▶" },
  { id:"hexeditor",  label:"Hex Editor",         icon:"≡" },
  { id:"analyzer",   label:"Protocol Analyzer",  icon:"〜" },
  { id:"settings",   label:"Settings",           icon:"◎" },
  { id:"firmware",   label:"Firmware",           icon:"↑" },
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
        color: active===item.id ? "var(--green)" : "var(--text-3)",
        border: active===item.id ? "1px solid var(--border)" : "1px solid transparent",
        filter: !connected && item.id !== "dashboard" ? "opacity(0.4)" : "none",
      }}>
        {item.icon}
        <span className="tooltip">{item.label}</span>
      </div>
    ))}
  </div>
);

export default Sidebar;
