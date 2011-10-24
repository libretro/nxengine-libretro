VERSION 5.00
Begin VB.Form frmTileKey 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "TileKey"
   ClientHeight    =   7455
   ClientLeft      =   45
   ClientTop       =   375
   ClientWidth     =   10095
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   7455
   ScaleWidth      =   10095
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdQuit 
      Caption         =   "&Quit"
      Height          =   495
      Left            =   8640
      TabIndex        =   4
      Top             =   240
      Width           =   1215
   End
   Begin VB.CommandButton cmdSolid 
      Caption         =   "&Solid"
      Height          =   495
      Left            =   8640
      TabIndex        =   3
      Top             =   1080
      Width           =   1215
   End
   Begin VB.CommandButton cmdSave 
      Caption         =   "&Save"
      Height          =   495
      Left            =   7200
      TabIndex        =   2
      Top             =   240
      Width           =   1215
   End
   Begin VB.CheckBox chkFlags 
      Caption         =   "Check1"
      Height          =   615
      Index           =   0
      Left            =   6480
      TabIndex        =   1
      Top             =   960
      Width           =   3495
   End
   Begin VB.ListBox lstTileNo 
      BeginProperty Font 
         Name            =   "Lucida Console"
         Size            =   9
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   5820
      Left            =   120
      MultiSelect     =   2  'Extended
      TabIndex        =   0
      Top             =   960
      Width           =   6135
   End
   Begin VB.Label Label2 
      Caption         =   $"frmTileKey.frx":0000
      Height          =   495
      Left            =   120
      TabIndex        =   6
      Top             =   6960
      Width           =   6495
   End
   Begin VB.Label Label1 
      Caption         =   $"frmTileKey.frx":00BE
      Height          =   735
      Left            =   120
      TabIndex        =   5
      Top             =   120
      Width           =   6135
   End
End
Attribute VB_Name = "frmTileKey"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Dim TFlags(32) As String
Dim nflags As Integer
Dim TileKey(256) As Long
Dim BitToMask(32) As Long

Dim curtile As Integer
Dim fname$
Dim needtosave As Integer

Sub LoadKey(keyname$)
    fname$ = keyname$
    Open fname$ For Binary As #1
    
    For i = 0 To 256
    
        TileKey(i) = 0
        mask = 1
        For curbyt = 0 To 3
            a$ = " ": Get #1, , a$
            byt = Asc(a$)
            
            bmask = 1
            For bt = 0 To 7
                isset = byt And bmask
                If isset <> 0 Then
                    TileKey(i) = TileKey(i) Or mask
                End If
                
                mask = mask * 2
                bmask = bmask * 2
            Next
        Next
    Next
    
    Close
End Sub

Sub SaveKey()
Dim byt(4) As Integer
Dim curbyt As Integer
Dim startbit As Integer
Dim mask As Integer

    Open fname$ For Output As #1
    
    For i = 0 To 256
        byt(0) = 0: byt(1) = 0: byt(2) = 0: byt(3) = 0
        
        startbit = 0
        For curbyt = 0 To 3
            mask = 1
            For b = startbit To startbit + 7
                bt = TileKey(i) And BitToMask(b)
                If bt <> 0 Then
                    byt(curbyt) = byt(curbyt) Or mask
                End If
                mask = mask * 2
            Next
            startbit = startbit + 8
        Next
        
        For curbyt = 0 To 3
            Print #1, Chr$(byt(curbyt));
        Next
    Next
    
    Beep
    needtosave = 0
    Close
End Sub


Private Sub chkFlags_Click(Index As Integer)
    needtosave = 1
    bv = BitToMask(Index)
    
    For curtile = 0 To lstTileNo.ListCount - 1
        If lstTileNo.Selected(curtile) Then
            TileKey(curtile) = TileKey(curtile) Or bv
            If chkFlags(Index).Value = False Then TileKey(curtile) = TileKey(curtile) Xor bv
            
            UpdTileInList curtile
        End If
    Next
    
    UpdFlagDisplay
    lstTileNo.SetFocus
End Sub

Private Sub cmdQuit_Click()
    If needtosave = 1 Then
        ok = MsgBox("Save?", vbYesNo, "")
        If ok = vbYes Then
            SaveKey
        End If
    End If
    
    End
End Sub

Private Sub cmdSave_Click()
    SaveKey
End Sub

Private Sub cmdSolid_Click()
    k = chkFlags(0).Value Xor 1
    For i = 0 To 2
        chkFlags(i).Value = k
    Next
End Sub

Private Sub Form_Load()
Dim mask As Long

    TFlags(0) = "SOLID_PLAYER"
    TFlags(1) = "SOLID_NPC"
    TFlags(2) = "SOLID_SHOT"
    TFlags(3) = "RESERVED1"
    TFlags(4) = "SPIKES"
    TFlags(5) = "FOREGROUND"
    TFlags(6) = "DESTROYABLE"
    TFlags(7) = "WATER"
    
    TFlags(8) = "CURRENT"
    TFlags(9) = "SLOPE"
    TFlags(10) = ""
    TFlags(11) = ""
    
    nflags = 12
    
    LoadKey "d:\dev\nx\tilekey.dat"
    
    mask = 1
    For i = 0 To 24
        BitToMask(i) = mask
        mask = mask * 2
    Next
    
    For i = 1 To nflags - 1
        Load chkFlags(i)
    Next
    
    lstTileNo.Clear
    For i = 0 To 255
        lstTileNo.AddItem "&"
    Next
    lstTileNo.ListIndex = 0
    lstTileNo.Selected(0) = True
    
    For i = 0 To 255: UpdTileInList i: Next
        
    UpdFlagDisplay
    Move (Screen.Width - Me.Width) / 2, (Screen.Height - Me.Height) / 2
End Sub

Sub UpdTileInList(i)
    a$ = " " + Right$("0" + Hex$(i), 2) + "   "
    
    bt = TileKey(i) And 7
    If bt = 7 Then
        a$ = a$ + "SOLID  "
        start = 3
    Else
        start = 0
    End If
    
    For bc = start To 24
        bt = TileKey(i) And BitToMask(bc)
        If bt <> 0 Then
            a$ = a$ + TFlags(bc) + "  "
        End If
    Next
    
    lstTileNo.List(i) = a$
End Sub

Sub UpdFlagDisplay()

oldneedtosave = needtosave
    curtile = lstTileNo.ListIndex
    
    For i = 0 To nflags - 1
        chkFlags(i).Caption = TFlags(i)
        If i > 0 Then
            chkFlags(i).Top = chkFlags(i - 1).Top + 480
            chkFlags(i).Visible = True
        End If
        
        bt = TileKey(curtile) And BitToMask(i)
        If bt <> 0 Then bt = 1 Else bt = 0
        
        chkFlags(i).Value = bt
        chkFlags(i).FontBold = bt
    Next
needtosave = oldneedtosave
End Sub

Private Sub lstTileNo_Click()
    Me.Caption = "TileKey - " + fname$ + " - Tile " + Right$("0" + Hex$(lstTileNo.ListIndex), 2)
    'Me.Caption = "TileKey - Tile " + Right$("0" + Hex$(lstTileNo.ListIndex), 2)
    UpdFlagDisplay
End Sub
