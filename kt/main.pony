use "collections"
use "logger"
use "net"

class HandleUDPNotify is UDPNotify
  let hmap: HashMap[String, String, HashEq[String]] = HashMap[String, String, HashEq[String]].create()

  fun ref not_listening(sock: UDPSocket ref): None val =>
    None

  fun ref received(sock: UDPSocket ref, data: Array[U8] iso, from: NetAddress) =>
    let src = recover val consume data end
    if src.size() == 0 then
      return
    end
    var cmd = String.from_array(recover val src.slice(0, 1) end)
    var key : String = ""
    var value : String= ""

    if src.size() > 1 then
      try
        key = String.from_array(recover val src.slice(1, src.find(0, 0)) end)
        if src.size() > (2 + key.size()) then
          value = String.from_array(recover val src.slice(key.size() + 2) end)
        end
      end
    end

    var result: String = ""
    match cmd
    | "G" => try result = hmap(key) end
    | "A" => try hmap.insert_if_absent(key, value) end
    | "S" => try hmap.insert(key, value) end
    | "D" => try hmap.remove(key) end
    else
      result = hmap.size().string()
    end

    var rsp: Array[U8] iso = recover iso Array[U8].create(2 + key.size() + result.size()) end
    rsp.append(cmd.lower())
    rsp.append(key)
    rsp.push(0)
    rsp.append(result)
    sock.write(consume rsp, from)


actor Main
  new create(env: Env) =>
    let logger = StringLogger(Info, env.out)
    logger.log("Started...\n")
    try
      let socket = UDPSocket(env.root as AmbientAuth, HandleUDPNotify, "", "6347")
    end
