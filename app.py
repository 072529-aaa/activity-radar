#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
活动雷达 ActivityRadar - Python GUI 前端
依赖: Python 3.x (Tkinter 内置)
运行: python app.py
"""

import tkinter as tk
from tkinter import ttk, messagebox
import subprocess
import json
import os
import sys
import urllib.request
import platform

APP_TITLE = "活动雷达 ActivityRadar"
APP_VERSION = "2.0 (C++ Engine + Python GUI)"
ENGINE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "activity_engine.exe")

CITIES = ["武汉", "北京", "上海", "广州", "深圳", "杭州", "成都", "南京", "长沙", "重庆", "合肥", "厦门", "全部"]

COLORS = {
    "bg": "#f9f9f7",
    "card": "#ffffff",
    "text": "#1c1c1c",
    "text_secondary": "#5c5c5c",
    "text_muted": "#9a9a9a",
    "border": "#e8e8e5",
    "accent": "#c96442",
    "ai": "#4a7ab8",
    "run": "#d97757",
    "vol": "#b8922a",
    "wuhan": "#b84242",
}


class ActivityRadarApp:
    def __init__(self, root):
        self.root = root
        self.root.title(APP_TITLE)
        self.root.geometry("960x680")
        self.root.minsize(800, 560)
        self.root.configure(bg=COLORS["bg"])
        self.current_city = tk.StringVar(value="武汉")
        self.current_type = tk.StringVar(value="all")
        self.search_var = tk.StringVar()
        self.sort_var = tk.StringVar(value="date")
        self.volunteer_only = tk.BooleanVar(value=False)
        self.activities = []
        self._setup_styles()
        self._build_ui()
        self._refresh()

    def _setup_styles(self):
        style = ttk.Style()
        try:
            style.theme_use("clam")
        except:
            pass
        style.configure("TFrame", background=COLORS["bg"])
        style.configure("Card.TFrame", background=COLORS["card"])
        style.configure("TLabel", background=COLORS["bg"], foreground=COLORS["text"], font=("Microsoft YaHei", 10))
        style.configure("Title.TLabel", background=COLORS["bg"], foreground=COLORS["text"], font=("Microsoft YaHei", 16, "bold"))
        style.configure("Muted.TLabel", background=COLORS["bg"], foreground=COLORS["text_muted"], font=("Microsoft YaHei", 9))
        style.configure("CardTitle.TLabel", background=COLORS["card"], foreground=COLORS["text"], font=("Microsoft YaHei", 11, "bold"))
        style.configure("CardMeta.TLabel", background=COLORS["card"], foreground=COLORS["text_secondary"], font=("Microsoft YaHei", 9))
        style.configure("TButton", font=("Microsoft YaHei", 9), padding=6)
        style.configure("Accent.TButton", font=("Microsoft YaHei", 9, "bold"), padding=6)
        style.configure("TCombobox", font=("Microsoft YaHei", 9))
        style.configure("TEntry", font=("Microsoft YaHei", 9))
        style.configure("TCheckbutton", background=COLORS["bg"], font=("Microsoft YaHei", 9))

    def _build_ui(self):
        header = ttk.Frame(self.root, style="TFrame")
        header.pack(fill="x", padx=20, pady=(16, 8))
        title_frame = ttk.Frame(header, style="TFrame")
        title_frame.pack(side="left")
        ttk.Label(title_frame, text="活动雷达", style="Title.TLabel").pack(anchor="w")
        ttk.Label(title_frame, text=f"ActivityRadar · {APP_VERSION}", style="Muted.TLabel").pack(anchor="w")
        loc_btn = ttk.Button(header, text="定位我的城市", command=self._geolocate, style="Accent.TButton")
        loc_btn.pack(side="right", padx=(8, 0))
        filter_bar = ttk.Frame(self.root, style="TFrame")
        filter_bar.pack(fill="x", padx=20, pady=8)
        ttk.Label(filter_bar, text="城市:").pack(side="left")
        city_combo = ttk.Combobox(filter_bar, textvariable=self.current_city, values=CITIES, width=8, state="readonly")
        city_combo.pack(side="left", padx=(4, 12))
        city_combo.bind("<<ComboboxSelected>>", lambda e: self._refresh())
        ttk.Label(filter_bar, text="类型:").pack(side="left")
        type_combo = ttk.Combobox(filter_bar, textvariable=self.current_type, values=["全部", "AI活动", "马拉松"], width=8, state="readonly")
        type_combo.pack(side="left", padx=(4, 12))
        type_combo.bind("<<ComboboxSelected>>", lambda e: self._refresh())
        ttk.Label(filter_bar, text="搜索:").pack(side="left")
        search_entry = ttk.Entry(filter_bar, textvariable=self.search_var, width=20)
        search_entry.pack(side="left", padx=(4, 12))
        search_entry.bind("<Return>", lambda e: self._refresh())
        ttk.Checkbutton(filter_bar, text="仅看志愿者招募", variable=self.volunteer_only, command=self._refresh).pack(side="left", padx=(0, 12))
        ttk.Label(filter_bar, text="排序:").pack(side="left")
        sort_combo = ttk.Combobox(filter_bar, textvariable=self.sort_var, values=["按时间", "武汉优先", "志愿者优先"], width=10, state="readonly")
        sort_combo.pack(side="left", padx=(4, 0))
        sort_combo.bind("<<ComboboxSelected>>", lambda e: self._refresh())
        self.stats_label = ttk.Label(self.root, text="", style="Muted.TLabel")
        self.stats_label.pack(fill="x", padx=20, pady=(4, 8))
        list_container = ttk.Frame(self.root, style="TFrame")
        list_container.pack(fill="both", expand=True, padx=20, pady=(0, 16))
        self.canvas = tk.Canvas(list_container, bg=COLORS["bg"], highlightthickness=0)
        scrollbar = ttk.Scrollbar(list_container, orient="vertical", command=self.canvas.yview)
        self.scroll_frame = ttk.Frame(self.canvas, style="TFrame")
        self.scroll_frame.bind("<Configure>", lambda e: self.canvas.configure(scrollregion=self.canvas.bbox("all")))
        self.canvas.create_window((0, 0), window=self.scroll_frame, anchor="nw", width=900)
        self.canvas.configure(yscrollcommand=scrollbar.set)
        self.canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        self.canvas.bind_all("<MouseWheel>", lambda e: self.canvas.yview_scroll(int(-1 * (e.delta / 120)), "units"))
        status_bar = ttk.Frame(self.root, style="TFrame")
        status_bar.pack(fill="x", padx=20, pady=(0, 10))
        self.status_label = ttk.Label(status_bar, text="就绪", style="Muted.TLabel")
        self.status_label.pack(side="left")
        ttk.Label(status_bar, text="数据仅供参考，以官方发布为准", style="Muted.TLabel").pack(side="right")

    def _query_engine(self):
        type_map = {"全部": "all", "AI活动": "ai", "马拉松": "marathon"}
        sort_map = {"按时间": "date", "武汉优先": "wuhan", "志愿者优先": "volunteer"}
        args = [ENGINE_PATH, "--json", "--city", self.current_city.get(), "--type", type_map.get(self.current_type.get(), "all"), "--sort", sort_map.get(self.sort_var.get(), "date")]
        if self.search_var.get().strip():
            args.extend(["--search", self.search_var.get().strip()])
        if self.volunteer_only.get():
            args.append("--volunteer")
        try:
            result = subprocess.run(args, capture_output=True, text=True, encoding="utf-8", timeout=10)
            if result.returncode == 0 and result.stdout.strip():
                return json.loads(result.stdout)
        except FileNotFoundError:
            self.status_label.config(text="警告: 未找到 C++ 引擎，使用 Python 内置数据")
        except Exception as e:
            self.status_label.config(text=f"引擎调用失败: {e}")
        return self._python_fallback()

    def _python_fallback(self):
        try:
            data_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data", "activities.json")
            if os.path.exists(data_file):
                with open(data_file, "r", encoding="utf-8") as f:
                    all_data = json.load(f)
            else:
                all_data = []
        except:
            all_data = []
        city = self.current_city.get()
        type_map = {"全部": "all", "AI活动": "ai", "马拉松": "marathon"}
        act_type = type_map.get(self.current_type.get(), "all")
        search = self.search_var.get().strip().lower()
        vol_only = self.volunteer_only.get()
        filtered = []
        for a in all_data:
            if city != "全部" and a.get("city") != city:
                continue
            if act_type != "all" and a.get("type") != act_type:
                continue
            if vol_only and not a.get("volunteer", {}).get("recruiting", False):
                continue
            if search:
                haystack = " ".join([a.get("title", ""), a.get("location", ""), a.get("organizer", ""), a.get("description", ""), " ".join(a.get("tags", []))]).lower()
                if search not in haystack:
                    continue
            filtered.append(a)
        sort_map = {"按时间": "date", "武汉优先": "wuhan", "志愿者优先": "volunteer"}
        sort_by = sort_map.get(self.sort_var.get(), "date")
        if sort_by == "wuhan":
            filtered.sort(key=lambda x: (not x.get("isWuhan", False), x.get("date", "")))
        elif sort_by == "volunteer":
            filtered.sort(key=lambda x: (not x.get("volunteer", {}).get("recruiting", False), x.get("date", "")))
        else:
            filtered.sort(key=lambda x: x.get("date", ""))
        return filtered

    def _refresh(self):
        self.status_label.config(text="正在搜索...")
        self.root.update()
        self.activities = self._query_engine()
        for widget in self.scroll_frame.winfo_children():
            widget.destroy()
        if not self.activities:
            ttk.Label(self.scroll_frame, text="\n未找到匹配的活动\n试试切换城市或调整筛选条件\n", style="Muted.TLabel", justify="center").pack(pady=40)
        else:
            for i, act in enumerate(self.activities):
                self._create_activity_card(act, i)
        ai_count = sum(1 for a in self.activities if a.get("type") == "ai")
        run_count = sum(1 for a in self.activities if a.get("type") == "marathon")
        vol_count = sum(1 for a in self.activities if a.get("volunteer", {}).get("recruiting", False))
        self.stats_label.config(text=f"共 {len(self.activities)} 场活动 · AI: {ai_count} · 马拉松: {run_count} · 招募志愿者: {vol_count}")
        self.status_label.config(text=f"搜索完成 · 当前城市: {self.current_city.get()}")

    def _create_activity_card(self, act, index):
        card = tk.Frame(self.scroll_frame, bg=COLORS["card"], highlightbackground=COLORS["border"], highlightthickness=1, bd=0)
        card.pack(fill="x", pady=4, padx=2)
        if act.get("isWuhan"):
            card.config(highlightbackground=COLORS["wuhan"])
        inner = tk.Frame(card, bg=COLORS["card"])
        inner.pack(fill="x", padx=14, pady=10)
        row1 = tk.Frame(inner, bg=COLORS["card"])
        row1.pack(fill="x")
        type_color = COLORS["ai"] if act.get("type") == "ai" else COLORS["run"]
        type_text = "AI活动" if act.get("type") == "ai" else "马拉松"
        tk.Label(row1, text=f" {type_text} · {act.get('category','')} ", bg=type_color, fg="white", font=("Microsoft YaHei", 8, "bold")).pack(side="left")
        if act.get("isWuhan"):
            tk.Label(row1, text=" 武汉 ", bg=COLORS["wuhan"], fg="white", font=("Microsoft YaHei", 8, "bold")).pack(side="left", padx=(6, 0))
        date_text = act.get("date", "")
        if act.get("endDate") and act.get("endDate") != act.get("date"):
            date_text += " ~ " + act.get("endDate", "")
        tk.Label(row1, text=date_text, bg=COLORS["card"], fg=COLORS["text_muted"], font=("Consolas", 9)).pack(side="right")
        title = act.get("title", "")
        tk.Label(inner, text=title, bg=COLORS["card"], fg=COLORS["text"], font=("Microsoft YaHei", 11, "bold"), anchor="w", justify="left").pack(fill="x", pady=(6, 2))
        loc_text = f"{act.get('city','')} · {act.get('location','')}"
        tk.Label(inner, text=loc_text, bg=COLORS["card"], fg=COLORS["text_secondary"], font=("Microsoft YaHei", 9), anchor="w").pack(fill="x")
        desc = act.get("description", "")
        if len(desc) > 80:
            desc = desc[:80] + "..."
        tk.Label(inner, text=desc, bg=COLORS["card"], fg=COLORS["text_muted"], font=("Microsoft YaHei", 9), anchor="w", justify="left", wraplength=850).pack(fill="x", pady=(4, 0))
        bottom = tk.Frame(inner, bg=COLORS["card"])
        bottom.pack(fill="x", pady=(8, 0))
        vol = act.get("volunteer", {})
        if vol.get("recruiting"):
            vol_text = f"● 招募志愿者 {vol.get('count',0)}人 (截止{vol.get('deadline','')})"
            tk.Label(bottom, text=vol_text, bg=COLORS["card"], fg=COLORS["vol"], font=("Microsoft YaHei", 9, "bold")).pack(side="left")
        else:
            tk.Label(bottom, text="暂无志愿者招募", bg=COLORS["card"], fg=COLORS["text_muted"], font=("Microsoft YaHei", 9)).pack(side="left")
        tk.Button(bottom, text="查看详情 →", command=lambda a=act: self._show_detail(a), bg=COLORS["card"], fg=COLORS["accent"], font=("Microsoft YaHei", 9), relief="flat", cursor="hand2", activebackground=COLORS["bg"]).pack(side="right")

    def _show_detail(self, act):
        win = tk.Toplevel(self.root)
        win.title(act.get("title", "活动详情"))
        win.geometry("560x520")
        win.configure(bg=COLORS["bg"])
        win.transient(self.root)
        win.grab_set()
        container = tk.Frame(win, bg=COLORS["bg"])
        container.pack(fill="both", expand=True, padx=20, pady=16)
        tk.Label(container, text=act.get("title", ""), bg=COLORS["bg"], fg=COLORS["text"], font=("Microsoft YaHei", 14, "bold"), wraplength=500, justify="left").pack(anchor="w", pady=(0, 8))
        info_frame = tk.Frame(container, bg=COLORS["bg"])
        info_frame.pack(fill="x", pady=8)
        info_items = [
            ("活动类型", f"{'AI活动' if act.get('type')=='ai' else '马拉松'} · {act.get('category','')}"),
            ("活动时间", f"{act.get('date','')} ~ {act.get('endDate','')}" if act.get('endDate') != act.get('date') else act.get('date', '')),
            ("活动地点", f"{act.get('city','')} · {act.get('location','')}"),
            ("主办方", act.get("organizer", "")),
        ]
        for i, (label, value) in enumerate(info_items):
            r, c = divmod(i, 2)
            cell = tk.Frame(info_frame, bg=COLORS["card"], highlightbackground=COLORS["border"], highlightthickness=1)
            cell.grid(row=r, column=c, sticky="nsew", padx=3, pady=3)
            tk.Label(cell, text=label, bg=COLORS["card"], fg=COLORS["text_muted"], font=("Microsoft YaHei", 8)).pack(anchor="w", padx=10, pady=(6, 0))
            tk.Label(cell, text=value, bg=COLORS["card"], fg=COLORS["text"], font=("Microsoft YaHei", 10, "bold"), wraplength=240, justify="left").pack(anchor="w", padx=10, pady=(0, 6))
        info_frame.grid_columnconfigure(0, weight=1)
        info_frame.grid_columnconfigure(1, weight=1)
        tk.Label(container, text="活动介绍", bg=COLORS["bg"], fg=COLORS["text"], font=("Microsoft YaHei", 10, "bold")).pack(anchor="w", pady=(8, 4))
        tk.Label(container, text=act.get("description", ""), bg=COLORS["bg"], fg=COLORS["text_secondary"], font=("Microsoft YaHei", 9), wraplength=500, justify="left").pack(anchor="w")
        tags = act.get("tags", [])
        if tags:
            tag_frame = tk.Frame(container, bg=COLORS["bg"])
            tag_frame.pack(anchor="w", pady=8)
            for t in tags:
                tk.Label(tag_frame, text=f" {t} ", bg=COLORS["card"], fg=COLORS["text_secondary"], font=("Microsoft YaHei", 8), highlightbackground=COLORS["border"], highlightthickness=1).pack(side="left", padx=(0, 4))
        vol = act.get("volunteer", {})
        if vol.get("recruiting"):
            vol_frame = tk.Frame(container, bg="#fdf8ed", highlightbackground=COLORS["vol"], highlightthickness=1)
            vol_frame.pack(fill="x", pady=10)
            tk.Label(vol_frame, text="● 志愿者招募中", bg="#fdf8ed", fg=COLORS["vol"], font=("Microsoft YaHei", 11, "bold")).pack(anchor="w", padx=12, pady=(8, 4))
            for label, value in [("招募人数", f"{vol.get('count', 0)} 人"), ("报名截止", vol.get("deadline", ""))]:
                row = tk.Frame(vol_frame, bg="#fdf8ed")
                row.pack(fill="x", padx=12, pady=1)
                tk.Label(row, text=f"{label}: ", bg="#fdf8ed", fg=COLORS["text_muted"], font=("Microsoft YaHei", 9)).pack(side="left")
                tk.Label(row, text=value, bg="#fdf8ed", fg=COLORS["text"], font=("Microsoft YaHei", 9, "bold")).pack(side="left")
            roles = vol.get("roles", [])
            if roles:
                tk.Label(vol_frame, text="岗位: " + "、".join(roles), bg="#fdf8ed", fg=COLORS["text_secondary"], font=("Microsoft YaHei", 9), wraplength=480, justify="left").pack(anchor="w", padx=12, pady=(4, 2))
            benefits = vol.get("benefits", [])
            if benefits:
                tk.Label(vol_frame, text="福利: " + "、".join(benefits), bg="#fdf8ed", fg=COLORS["text_secondary"], font=("Microsoft YaHei", 9), wraplength=480, justify="left").pack(anchor="w", padx=12, pady=(0, 2))
            if vol.get("contact"):
                tk.Label(vol_frame, text="报名: " + vol["contact"], bg="#fdf8ed", fg=COLORS["text_secondary"], font=("Microsoft YaHei", 9), wraplength=480, justify="left").pack(anchor="w", padx=12, pady=(0, 8))
        tk.Button(container, text="关闭", command=win.destroy, bg=COLORS["text"], fg="white", font=("Microsoft YaHei", 10), relief="flat", cursor="hand2", padx=20, pady=6).pack(pady=(12, 0))

    def _geolocate(self):
        self.status_label.config(text="正在定位...")
        self.root.update()
        try:
            url = "http://ip-api.com/json/?lang=zh-CN&fields=city,lat,lon"
            req = urllib.request.Request(url, headers={"User-Agent": "ActivityRadar/2.0"})
            with urllib.request.urlopen(req, timeout=5) as resp:
                data = json.loads(resp.read().decode("utf-8"))
            city = data.get("city", "")
            for c in CITIES:
                if c in city or city in c:
                    self.current_city.set(c)
                    self.status_label.config(text=f"已定位到 {c}")
                    self._refresh()
                    return
            self.status_label.config(text=f"定位到 {city}，不在支持列表中，请手动选择")
        except Exception as e:
            self.status_label.config(text=f"定位失败: {e}，请手动选择城市")


def main():
    root = tk.Tk()
    app = ActivityRadarApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
