//
// Created by Brendan Berg on 2019-10-07.
//


#include <egc_sequencer/Sequencer.hpp>


egc::Sequencer::Sequencer (std::shared_ptr<egc::Memory> memory)
    : m_Memory(std::move(memory))
{ }


void egc::Sequencer::Step ()
{
    unsigned short z = m_Memory->Read(00005u) & 007777u;
    unsigned short instruction = m_Memory->Read(z) & 077777u;
    m_Memory->Write(00005u, z + 000001u);

    unsigned short opcode = instruction & 070000u;
    unsigned short quartercode = instruction & 006000u;

    switch (opcode)
    {
        case 020000u:
            switch (quartercode)
            {
                case 004000u:
                    INCR(instruction);
                    break;
                case 006000u:
                    ADS(instruction);
                    break;
                default:
                    break;
            }
            break;
        case 050000u:
            switch (quartercode)
            {
                case 004000u:
                    TS(instruction);
                    break;
                default:
                    break;
            }
            break;
        case 060000u:
            AD(instruction);
            break;
        default:
            break;
    }
}


std::pair<unsigned short, unsigned short> egc::Sequencer::AddWords (unsigned short a, unsigned short b)
{
    unsigned short signA = a & 040000u;
    unsigned short signB = b & 040000u;

    unsigned short result = a + b;
    unsigned short overflow = 000000;
    unsigned short carry = (result & 0100000u) >> 15;
    result &= 077777u;
    result += carry;

    unsigned short signResult = (result & 040000u);
    if (signA == signB && signA != signResult)
    {
        result &= 037777u;
        result |= signA;

        if (signA == 040000u)
        {
            overflow = 077776u;
        }
        else
        {
            overflow = 000001u;
        }
    }

    result &= 077777u;

    return std::make_pair(result, overflow);
}


void egc::Sequencer::AD (unsigned short instruction)
{
    unsigned short k = instruction & 007777u;

    auto mk = m_Memory->Read(k);
    auto a = m_Memory->Read(00000u);

    auto [result, overflow] = AddWords(a, mk);

    m_Memory->Write(k, mk);
    m_Memory->Write(00000u, result);

    m_Memory->SetAccumulatorOverflow(overflow);
}


void egc::Sequencer::ADS (unsigned short instruction)
{
    unsigned short k = instruction & 001777u;

    auto mk = m_Memory->Read(k);
    auto a = m_Memory->Read(00000u);

    auto [result, overflow] = AddWords(a, mk);

    m_Memory->Write(k, result);
    m_Memory->Write(00000u, result);

    m_Memory->SetAccumulatorOverflow(overflow);
}


void egc::Sequencer::INCR (unsigned short instruction)
{
    unsigned short k = instruction & 001777u;

    auto mk = m_Memory->Read(k);

    unsigned short sign = mk & 040000u;

    if (mk == 077777u)
    {
        mk = 000001u;
    }
    else
    {
        mk += 000001u;
        if (sign != 040000u)
        {
            mk &= 037777u;
        }
    }

    m_Memory->Write(k, mk);
}


void egc::Sequencer::TS (unsigned short instruction)
{
    unsigned short k = instruction & 001777u;

    if (k == 000000u) // OVSK
    {
        auto overflow = m_Memory->GetAccumulatorOverflow();

        if (overflow)
        {
            m_Memory->Write(00005u, m_Memory->Read(00005u) + 000001u);
        }
    }
    else
    {
        auto a = m_Memory->Read(00000u);

        auto overflow = m_Memory->GetAccumulatorOverflow();

        m_Memory->Write(k, a);

        if (overflow)
        {
            m_Memory->Write(00000u, overflow);
            m_Memory->Write(00005u, m_Memory->Read(00005u) + 000001u);
        }
        else
        {
            m_Memory->Write(00000u, a);
        }
    }
}
