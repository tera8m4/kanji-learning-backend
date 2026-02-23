import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import './index.css'
import App from './components/App.tsx'
import { createTransport } from './core/transportFactory'

const transport = createTransport()

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <App transport={transport} />
  </StrictMode>,
)
