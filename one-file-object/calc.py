from textual.app import App, ComposeResult
from textual.containers import Container, Horizontal, ScrollableContainer
from textual.widgets import Static, Button, Digits
from textual.reactive import reactive

class BitApp(App):
    """比特网格编辑器 - 使用按钮版本"""
    
    CSS = """
    Screen {
        background: #1e1e1e;
        color: #ffffff;
    }
    
    .main-container {
        width: 99;
        height: 24;
        overflow: auto;
    }
    
    .title {
        text-align: center;
        text-style: bold;
        color: #4ec9b0;
        margin: 1;
    }
    
    .value-display {
        background: #2d2d30;
        color: #ce9178;
        /*text-align: center;*/
        padding: 0;
        /*border: solid #666;*/
        margin: 1;
    }
    
    .bit-grid {
        /*border: solid #666;*/
        margin: 1;
    }
    
    .bit-row {
        margin: 0;
    }
    
    .bit-item {
        margin: 0 1;
    }
    
    .bit-position {
        color: #888;
        text-align: center;
    }
    
    .bit-button {
        background: #2d2d30;
        text-align: left;
        border: none;
        /*border: solid #444;*/
    }
    
    .bit-button.bit-set {
        background: #007acc;
        color: white;
    }
    
    .footer {
        text-align: center;
        color: #888;
        margin: 1;
    }
    """
    
    current_value = reactive(0)
    
    def compose(self) -> ComposeResult:
        with ScrollableContainer(classes="main-container"):
            yield Static("比特网格编辑器", classes="title")
            yield Digits("0", classes="value-display", id="dec-display")
            
            with Container(classes="bit-grid"):
                # 第1行：比特25-32
                with Horizontal(classes="bit-row"):
                    for i in range(32, 24, -1):
                        with Container(classes="bit-item"):
                            yield Static(f"{i-1}", classes="bit-position")
                            yield Button("0    ", classes="bit-button", id=f"bit-{i}")
                
                # 第2行：比特17-24
                with Horizontal(classes="bit-row"):
                    for i in range(24, 16, -1):
                        with Container(classes="bit-item"):
                            yield Static(f"{i-1}", classes="bit-position")
                            yield Button("0    ", classes="bit-button", id=f"bit-{i}")
                
                # 第3行：比特9-16
                with Horizontal(classes="bit-row"):
                    for i in range(16, 8, -1):
                        with Container(classes="bit-item"):
                            yield Static(f"{i-1}", classes="bit-position")
                            yield Button("0    ", classes="bit-button", id=f"bit-{i}")
                
                # 第4行：比特1-8
                with Horizontal(classes="bit-row"):
                    for i in range(8, 0, -1):
                        with Container(classes="bit-item"):
                            yield Static(f"{i-1}", classes="bit-position")
                            yield Button("0    ", classes="bit-button", id=f"bit-{i}")
            
            #yield Static("点击比特切换 0/1 • 按 C 清除 • 按 ESC 退出", classes="footer")
    
    def on_mount(self):
        self.update_display()
    
    def watch_current_value(self, value):
        self.update_display()
    
    def on_button_pressed(self, event):
        """处理按钮点击事件"""
        button = event.button
        if button and button.id and button.id.startswith("bit-"):
            bit_pos = int(button.id.replace("bit-", ""))
            actual_bit_pos = bit_pos - 1
            self.current_value ^= (1 << actual_bit_pos)
            # 限制为32位
            self.current_value &= 0xFFFFFFFF
    
    def update_display(self):
        """更新显示"""
        # 将32位无符号整数转换为有符号整数
        if self.current_value & 0x80000000:  # 检查第31位（符号位）
            signed_value = self.current_value - 0x100000000
        else:
            signed_value = self.current_value
        
        self.query_one("#dec-display").update(str(signed_value))
        
        for i in range(1, 33):
            try:
                button = self.query_one(f"#bit-{i}")
                bit_value = (self.current_value >> (i - 1)) & 1 # 取当前值的二进制最后一位
                button.label = str(bit_value) + "    "
                
                if bit_value:
                    button.add_class("bit-set")
                else:
                    button.remove_class("bit-set")
            except:
                pass
    
    def on_key(self, event):
        if event.key == "escape":
            self.exit()
        elif event.key == "c":
            self.current_value = 0
        elif event.key == "d":
            self.current_value = 2147483647

if __name__ == "__main__":
    app = BitApp()
    app.run()