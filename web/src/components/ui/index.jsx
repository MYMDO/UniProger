/* ── UI Primitives ──────────────────────────────────────────────────────── */

export const Badge = ({ color, children, small }) => (
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

export const Dot = ({ color, pulse }) => (
  <span style={{
    width:7, height:7, borderRadius:"50%",
    background: color || "var(--green)",
    display:"inline-block", flexShrink:0,
    animation: pulse ? "pulse-green 1.8s ease-in-out infinite" : "none",
    boxShadow: `0 0 6px ${color || "var(--green)"}`
  }} />
);

export const Btn = ({ children, variant="primary", onClick, disabled, small, icon, style={} }) => {
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

export const Card = ({ children, style={}, glow }) => (
  <div style={{
    background:"var(--bg-surface)", border:`1px solid ${glow?"var(--border)":"var(--border-dim)"}`,
    borderRadius:"var(--r-lg)", padding:"16px",
    boxShadow: glow ? "0 0 24px var(--green-glow)" : "none",
    ...style
  }}>{children}</div>
);

export const ProgressBar = ({ value, color="var(--green)", animated }) => (
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

export const SectionTitle = ({ children, accent }) => (
  <div style={{display:"flex", alignItems:"center", gap:8, marginBottom:12}}>
    {accent && <div style={{width:3,height:14,background:"var(--green)",borderRadius:2,boxShadow:"0 0 6px var(--green)"}} />}
    <span style={{fontFamily:"var(--font-display)", fontSize:11, fontWeight:700, letterSpacing:"0.12em", textTransform:"uppercase", color:"var(--text-2)"}}>{children}</span>
  </div>
);
